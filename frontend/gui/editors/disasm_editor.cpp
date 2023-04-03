
#include "disasm_editor.h"
#include "imgui.h"
#include "session.h"
#include "config.h"
#include "c65c02.h"
#include "dis6502.h"
#include <fmt/core.h>
#include "bootstrap-icons.h"

DisasmEditor::DisasmEditor(int id)
{
    _id = id;
    _identifier = fmt::format("Disassembly {}", id);
}

DisasmEditor::~DisasmEditor()
{
    Config::getInstance().delete_disasm_editor(_session->identifier(), this);
}

void DisasmEditor::set_session(std::shared_ptr<Session> session)
{
    _session = session;
    Config::getInstance().load_disasm_editor(_session->identifier(), this);
}

int DisasmEditor::id()
{
    return _id;
}

std::string DisasmEditor::identifer()
{
    return _identifier;
}

bool DisasmEditor::enabled()
{
    return _session != nullptr;
}

bool DisasmEditor::is_read_only()
{
    if (!enabled())
    {
        return true;
    }

    return _session->status() != SessionStatus_Break;
}

void DisasmEditor::draw_contents()
{
    if (!enabled())
    {
        return;
    }

    ImGui::AlignTextToFramePadding();

    draw_disasm_table();
    draw_options();
}

void DisasmEditor::draw_options()
{
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (int)ImGuiStyleVar_CellPadding - 10);
    ImGui::Separator();
    if (ImGui::Button("Options"))
    {
        ImGui::OpenPopup("disasmcontext");
    }
    if (ImGui::BeginPopup("disasmcontext"))
    {
        ImGui::Checkbox("Follow PC", &_follow_pc);
        ImGui::Checkbox("Show labels", &_show_labels);
        ImGui::EndPopup();
    }
    ImGui::SameLine();
    ImGui::Text("Jump to");
    ImGui::SameLine();
    ImGui::BeginDisabled(_follow_pc);
    ImGui::SetNextItemWidth(120);
    if (imgui_autocomplete_input("##disasmtableaddr", _address_buf, sizeof(_address_buf), _session->symbols().overrides(), ImGuiInputTextFlags_None) && _address_buf[0])
    {
        if (strlen(_address_buf) <= 0 || is_read_only())
        {
            return;
        }

        int addr = _session->symbols().get_addr(std::string(_address_buf));

        if (addr < 0)
        {
            std::from_chars(_address_buf, _address_buf + sizeof(_address_buf) - 1, addr, 16);
        }

        _local_pc = (uint16_t)addr;

        memset(_address_buf, 0, sizeof(_address_buf));
    }

    ImGui::EndDisabled();
}

void DisasmEditor::draw_disasm_table()
{
    float rowHeight = ImGui::GetTextLineHeight() + ImGui::GetStyle().CellPadding.y * 2;

    auto tableSize = ImGui::GetWindowSize();

    tableSize.y -= (ImGuiStyleVar_CellPadding + 10 + ImGui::GetFrameHeight()); // options height

    int itemCount = (int)(tableSize.y / rowHeight) + 1;

    C6502_REGS regs;
    _session->system()->GetRegs(regs);

    if (_follow_pc)
    {
        _local_pc = regs.PC;
    }

    int working_pc = _local_pc;

    ImGui::BeginTable("##DisasmTable", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp, tableSize);

    ImGui::TableSetupColumn("##disasmtableaddr", ImGuiTableColumnFlags_NoHeaderLabel, 0.20F);
    ImGui::TableSetupColumn("##disasmtablehex", ImGuiTableColumnFlags_NoHeaderLabel, 0.25F);
    ImGui::TableSetupColumn("##disasmtableopc", ImGuiTableColumnFlags_NoHeaderLabel, 0.15F);
    ImGui::TableSetupColumn("##disasmtableope", ImGuiTableColumnFlags_NoHeaderLabel, 0.40F);

    if (working_pc)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(" ");
        if (ImGui::IsItemVisible())
        {
            scroll_up();
        }
        ImGui::TableNextRow();
    }

    int rows = 0;

    auto &bps = _session->breakpoints();

    do
    {
        ImGui::TableNextRow();

        auto entry = disassemble(working_pc);

        entry.is_pc = entry.base_address == regs.PC;
        entry.has_breakpoint = std::any_of(bps.begin(), bps.end(), [entry](const Breakpoint &bp) { return bp.enabled && bp.address == entry.base_address; });

        rows = draw_disasm_entry(entry);

        working_pc += entry.data_length;

        if (working_pc < 0xffff && !itemCount && ImGui::IsItemVisible())
        {
            scroll_down();
        }

        itemCount -= rows;

    } while (itemCount >= 0);

    ImGui::SetScrollY(rowHeight);

    ImGui::EndTable();
}

int DisasmEditor::draw_disasm_entry(DisasmEntry &entry)
{
    int itemcount = 1;

    if (!entry.base_address_symbol.empty())
    {
        auto col = ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text));

        ImGui::TableNextColumn();
        auto cursor = ImGui::GetCursorScreenPos();

        ImGui::Text(" ");
        ImGui::TableNextRow();

        auto drawList = ImGui::GetWindowDrawList();

        ImVec2 rectMin = cursor;
        ImVec2 rectMax(cursor.x + 200, cursor.y + ImGui::GetTextLineHeight());

        ImGui::PushClipRect(rectMin, rectMax, false);

        drawList->AddText(cursor, col, fmt::format("{}:", entry.base_address_symbol).c_str());

        ImGui::PopClipRect();
        ++itemcount;
    }

    bool selected = false;
    ImGui::TableNextColumn();
    if (ImGui::Selectable(fmt::format("{}{}{:04X}:", entry.has_breakpoint ? "*" : " ", entry.is_pc ? ">" : " ", entry.base_address).c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
    {
        if (ImGui::IsMouseDoubleClicked(0))
        {
            _session->toggle_breakpoint(entry.base_address);
        }
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.data.c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.opcode.c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.operands.c_str());

    return itemcount;
}

DisasmEntry DisasmEditor::disassemble(uint16_t addr)
{
    DisasmEntry ret{};

    auto system = _session->system();
    uint16_t opcode, operand;
    uint16_t count = next_address(addr) - addr;

    ret.data_length = count;

    ret.base_address = addr;

    if (_show_labels)
    {
        auto &sym = _session->symbols().get_symbol(addr);

        if (sym.override)
        {
            ret.base_address_symbol = sym.symbol;
        }
    }

    for (int loop = 0; loop < count; loop++)
    {
        ret.data += fmt::format("{:02X} ", system->Peek_RAM(addr + loop));
    }

    opcode = system->Peek_RAM(addr);

    ret.opcode = mLookupTable[opcode].opcode;

    switch (count)
    {
    case 3:
        addr++;
        operand = system->Peek_RAM(addr++);
        operand += (system->Peek_RAM(addr++)) << 8;
        break;
    case 2:
        addr++;
        operand = system->Peek_RAM(addr++);
        break;
    case 1:
    default:
        addr++;
        break;
    }

    switch (mLookupTable[opcode].mode)
    {
    case accu:
        ret.operands = "A";
        break;
    case imm:
        ret.operands = fmt::format("#${:02X}", operand);
        break;
    case absl:
        if (_show_labels)
            ret.operands = _session->symbols().get_symbol(operand).symbol;
        else
            ret.operands = fmt::format("${:04X}", operand);
        break;
    case rel:
        int scrap;
        if (operand > 128)
            scrap = -128 + (operand & 0x7f);
        else
            scrap = operand;
        if (_show_labels)
            ret.operands = _session->symbols().get_symbol(addr + scrap).symbol;
        else
            ret.operands = fmt::format("${:04X}", addr + scrap);
        break;
    case iabs:
        if (_show_labels)
            ret.operands = fmt::format("({})", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("(${:04X})", operand);
        break;
    case ind:
        if (_show_labels)
            ret.operands = fmt::format("({})", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("(${:02X})", operand);
        break;
    case zp:
        if (_show_labels)
            ret.operands = _session->symbols().get_symbol(operand).symbol;
        else
            ret.operands = fmt::format("${:02X}", operand);
        break;
    case zpx:
        if (_show_labels)
            ret.operands = fmt::format("{},X", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("${:02X},X", operand);
        break;
    case zpy:
        if (_show_labels)
            ret.operands = fmt::format("{},Y", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("${:02X},Y", operand);
        break;
    case absx:
        if (_show_labels)
            ret.operands = fmt::format("{},X", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("${:04X},X", operand);
        break;
    case absy:
        if (_show_labels)
            ret.operands = fmt::format("{},Y", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("${:04X},Y", operand);
        break;
    case iabsx:
        if (_show_labels)
            ret.operands = fmt::format("({},X)", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("(${:04X},X)", operand);
        break;
    case zrel:
        scrap = operand >> 8;
        if (scrap > 128)
            scrap = -128 + (scrap & 0x7f);
        if (_show_labels)
            ret.operands = fmt::format("{},{}", _session->symbols().get_symbol(operand & 0xff).symbol, _session->symbols().get_symbol(addr + scrap).symbol);
        else
            ret.operands = fmt::format("${:02X},${:04X}", operand & 0xff, addr + scrap);
        break;
    case indx:
        if (_show_labels)
            ret.operands = fmt::format("({}),X", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("(${:02X}),X", operand);
        break;
    case indy:
        if (_show_labels)
            ret.operands = fmt::format("({}),Y", _session->symbols().get_symbol(operand).symbol);
        else
            ret.operands = fmt::format("(${:02X}),Y", operand);
        break;
    case impl:
    case illegal:
    default:
        break;
    }

    return ret;
}

int16_t DisasmEditor::next_address(int addr)
{
    if (addr > 0xffff)
    {
        addr = 0xffff;
    }

    UBYTE data = _session->system()->Peek_RAM(addr);

    int operand = mLookupTable[data].mode;

    addr = (addr + mOperandSizes[operand]);

    if (addr > 0xffff)
    {
        addr = 0xffff;
    }

    return addr;
}

void DisasmEditor::scroll_down()
{
    _local_pc = next_address(_local_pc);
}

void DisasmEditor::scroll_up()
{
    uint8_t data;
    int operand, size;
    uint16_t address = _local_pc;

    if (address > 0xffffU)
        address = 0xffffU;
    if (address < 4)
    {
        _local_pc = address;
        return;
    }

    for (int loop = 1; loop < 4; loop++)
    {
        data = _session->system()->Peek_RAM(address - loop);
        operand = mLookupTable[data].mode;
        size = mOperandSizes[operand];

        if (size == loop)
        {
            _local_pc = ((address - loop) & 0xffff);
            return;
        }
    }

    _local_pc = ((address - 1) & 0xffff);
}

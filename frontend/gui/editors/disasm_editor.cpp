
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
    Config::get_instance().delete_disasm_editor(_session->identifier(), this);
}

void DisasmEditor::set_session(std::shared_ptr<Session> session)
{
    _session = session;
    Config::get_instance().load_disasm_editor(_session->identifier(), this);
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
    auto font_size = ImGui::GetFontSize();

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - (int)ImGuiStyleVar_CellPadding - font_size);
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
    ImGui::SetNextItemWidth(font_size * 8);
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

    tableSize.y -= (ImGuiStyleVar_CellPadding + ImGui::GetFontSize() + ImGui::GetFrameHeight()); // options height

    int itemCount = (int)(tableSize.y / rowHeight) + 1;

    C6502_REGS regs;
    _session->system()->GetRegs(regs);

    if (_follow_pc)
    {
        _local_pc = regs.PC;
    }

    int working_pc = _local_pc;

    if (ImGui::BeginTable("##DisasmTable", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp, tableSize))
    {

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
            std::string empty;
            _session->toggle_breakpoint(entry.base_address, LynxMemBank_RAM, BreakPointType_EXEC, empty);
        }
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.data.c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.opcode.c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.operands.c_str());
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        auto tgt = get_opcode_target(entry);
        if (tgt >= 0)
        {
            ImGui::SetTooltip("0x%02X", tgt);
        }
    }

    return itemcount;
}

int16_t DisasmEditor::get_opcode_target(DisasmEntry &entry)
{
    auto system = _session->system();
    auto addr = entry.base_address;
    uint16_t opcode, operand;
    uint16_t count = next_address(addr) - addr;

    opcode = system->mRam->Peek(addr);

    switch (count)
    {
    case 3:
        addr++;
        operand = system->mRam->Peek(addr++);
        operand += (system->mRam->Peek(addr++)) << 8;
        break;
    case 2:
        addr++;
        operand = system->mRam->Peek(addr++);
        break;
    case 1:
    default:
        addr++;
        break;
    }

    int16_t ret = -1;

    switch (mLookupTable[opcode].mode)
    {
    case accu:
        break;
    case imm:
        break;
    case absl:
        ret = system->mRam->Peek(operand);
        break;
    case rel:
        int scrap;
        if (operand > 128)
            scrap = -128 + (operand & 0x7f);
        else
            scrap = operand;
        ret = system->mRam->Peek(addr + scrap);
        break;
    case iabs:
        ret = system->mRam->Peek(system->mRam->PeekW(operand));
        break;
    case ind:
        ret = system->mRam->Peek(system->mRam->Peek(operand));
        break;
    case zp:
        ret = system->mRam->Peek(operand);
        break;
    case zpx: {
        C6502_REGS regs;
        system->mCpu->GetRegs(regs);
        ret = system->mRam->Peek(operand + regs.X);
    }
    break;
    case zpy: {
        C6502_REGS regs;
        system->mCpu->GetRegs(regs);
        ret = system->mRam->Peek(operand + regs.Y);
    }
    break;
    case absx: {
        C6502_REGS regs;
        system->mCpu->GetRegs(regs);
        ret = system->mRam->Peek(operand + regs.X);
    }
    break;
    case absy: {
        C6502_REGS regs;
        system->mCpu->GetRegs(regs);
        ret = system->mRam->Peek(operand + regs.Y);
    }
    break;
    case iabsx: {
        C6502_REGS regs;
        system->mCpu->GetRegs(regs);
        ret = system->mRam->Peek(system->mRam->PeekW(operand + regs.X));
    }
    break;
    case zrel:
        scrap = operand >> 8;
        if (scrap > 128)
            scrap = -128 + (scrap & 0x7f);
        break;
    case indx: {
        C6502_REGS regs;
        system->mCpu->GetRegs(regs);
        ret = system->mRam->Peek(system->mRam->PeekW(operand) + regs.X);
    }
    break;
    case indy: {
        C6502_REGS regs;
        system->mCpu->GetRegs(regs);
        ret = system->mRam->Peek(system->mRam->PeekW(operand) + regs.Y);
    }
    break;
    case impl:
    case illegal:
    default:
        break;
    }

    return ret;
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

    opcode = system->mRam->Peek(addr);
    ret.data = fmt::format("{:02X}", opcode);
    ret.opcode = mLookupTable[opcode].opcode;

    switch (count)
    {
    case 3:
        addr++;
        operand = system->mRam->Peek(addr++);
        operand += (system->mRam->Peek(addr++)) << 8;
        ret.data += fmt::format(" {:02X} {:02X}", operand & 0xFF, operand >> 8);
        break;
    case 2:
        addr++;
        operand = system->mRam->Peek(addr++);
        ret.data += fmt::format(" {:02X}", operand);
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

    UBYTE data = _session->system()->mRam->Peek(addr);

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
        data = _session->system()->mRam->Peek(address - loop);
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

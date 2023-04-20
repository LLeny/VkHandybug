#include "breakpoints_editor.h"
#include "session.h"
#include "imgui.h"
#include <fmt/core.h>
#include "bootstrap-icons.h"

BreakpointsEditor::BreakpointsEditor()
{
}

BreakpointsEditor::~BreakpointsEditor()
{
}

void BreakpointsEditor::render()
{
    ImGui::AlignTextToFramePadding();

    render_header();

    ImGui::Separator();

    render_table();
}

BreakPointType BreakpointsEditor::type_get_type(const char *type)
{
    for (int i = 0; i < BreakPointType_MAX; ++i)
    {
        auto t = type_get_desc((BreakPointType)i);

        while (*type or *t)
        {
            char c = std::toupper(*type++);
            char d = std::toupper(*t++);
            if (c != d)
                continue;
        }
        return (BreakPointType)i;
    }
    return (BreakPointType)-1;
}

const char *BreakpointsEditor::type_get_desc(BreakPointType type) const
{
    const char *descs[] = {"Exec", "Read", "Write"};
    IM_ASSERT(type >= 0 && type < BreakPointType_MAX);
    return descs[type];
}

void BreakpointsEditor::render_header()
{
    ImGui::Text("Add breakpoint, Address");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);

    imgui_autocomplete_input("##bpaddr", _address_buf, sizeof(_address_buf), _session->symbols().overrides(), ImGuiInputTextFlags_None);

    ImGui::SameLine();
    ImGui::Text("Bank");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    if (ImGui::BeginCombo("##bpbank", mem_bank_get_desc(_mem_bank), ImGuiComboFlags_HeightLargest))
    {
        for (int n = 0; n < LynxMemBank_MAX; n++)
        {
            if (ImGui::Selectable(mem_bank_get_desc((LynxMemBank)n), _mem_bank == n))
            {
                _mem_bank = (LynxMemBank)n;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::Text("Type");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    if (ImGui::BeginCombo("##bptype", type_get_desc(_type), ImGuiComboFlags_HeightLargest))
    {
        for (int n = 0; n < BreakPointType_MAX; n++)
        {
            if (ImGui::Selectable(type_get_desc((BreakPointType)n), _type == n))
            {
                _type = (BreakPointType)n;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    if (!is_read_only() && ImGui::Button("Add"))
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

        _session->add_breakpoint((uint16_t)addr, _mem_bank, _type);

        memset(_address_buf, 0, sizeof(_address_buf));
    }
}

void BreakpointsEditor::render_table()
{
    if (ImGui::BeginTable("##bpitems", 6, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Del");
        ImGui::TableSetupColumn("E/D");
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Symbol");
        ImGui::TableSetupColumn("Bank");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (auto &bp : _session->breakpoints())
        {
            render_entry(bp);
        }
        ImGui::EndTable();
    }
}

void BreakpointsEditor::render_entry(Breakpoint &bp)
{
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(15);

    std::string lbi = fmt::format("{}##bi{}{}{}", BootstrapIcons_trash, bp.address, (int)bp.bank, (int)bp.type);
    if (ImGui::Button(lbi.c_str()))
    {
        _session->delete_breakpoint(bp.address, bp.bank, bp.type);
    }

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(15);
    std::string leni = fmt::format("{}##eni{}{}{}", bp.enabled ? BootstrapIcons_bookmark_plus : BootstrapIcons_bookmark_dash, bp.address, (int)bp.bank, (int)bp.type);
    if (ImGui::Button(leni.c_str()))
    {
        bp.enabled = !bp.enabled;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip(bp.enabled ? "Enabled" : "Disabled");
    }

    ImGui::TableNextColumn();
    std::string laddr = fmt::format("${:04X}", bp.address);
    ImGui::Text("%s", laddr.c_str());

    ImGui::TableNextColumn();
    auto sym = _session->symbols().get_symbol(bp.address);
    if (sym.override)
    {
        ImGui::Text("%s", sym.symbol.c_str());
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", mem_bank_get_desc(bp.bank));

    ImGui::TableNextColumn();
    ImGui::Text("%s", type_get_desc(bp.type));
}
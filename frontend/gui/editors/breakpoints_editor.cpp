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

void BreakpointsEditor::render_header()
{
    ImGui::Text("Add breakpoint, address");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);

    if (imgui_autocomplete_input("##bpaddr", _address_buf, sizeof(_address_buf), _session->symbols().overrides(), ImGuiInputTextFlags_None) && _address_buf[0])
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

        _session->add_breakpoint((uint16_t)addr);

        memset(_address_buf, 0, sizeof(_address_buf));
    }
}

void BreakpointsEditor::render_table()
{
    if (ImGui::BeginTable("##bpitems", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Del");
        ImGui::TableSetupColumn("E/D");
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Symbol");
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

    std::string lbi = fmt::format("{}##bi{}", BootstrapIcons_trash, bp.address);
    if (ImGui::Button(lbi.c_str()))
    {
        _session->delete_breakpoint(bp.address);
    }

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(15);
    std::string leni = fmt::format("{}##eni{}", bp.enabled ? BootstrapIcons_bookmark_plus : BootstrapIcons_bookmark_dash, bp.address);
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
}
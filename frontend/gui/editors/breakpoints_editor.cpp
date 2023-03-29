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
    char buf[10];

    ImGui::Text("Add breakpoint, address");

    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);

    if (ImGui::InputText("##bpaddr", buf, 6, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if (strlen(buf) <= 0 || is_read_only())
        {
            return;
        }

        int addr;
        std::from_chars(buf, buf + 5, addr, 16);

        _session->add_breakpoint(addr);
    }
}

void BreakpointsEditor::render_table()
{
    if (ImGui::BeginTable("##bpitems", 3, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Del");
        ImGui::TableSetupColumn("E/D");
        ImGui::TableSetupColumn("Address");
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
}
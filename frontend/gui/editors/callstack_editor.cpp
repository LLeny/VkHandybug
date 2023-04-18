
#include "callstack_editor.h"
#include "imgui.h"
#include "bootstrap-icons.h"

CallstackEditor::CallstackEditor()
{
}

CallstackEditor::~CallstackEditor()
{
}

void CallstackEditor::render()
{
    if (!enabled())
    {
        return;
    }

    if (ImGui::BeginTable("##Callstackitems", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Source");
        ImGui::TableSetupColumn(" ");
        ImGui::TableSetupColumn("Dest");
        ImGui::TableSetupColumn(" ");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (int i = _session->callstack().size() - 1; i >= 0; --i)
        {
            draw_callstack_entry(_session->callstack()[i]);
        }

        ImGui::EndTable();
    }
}

void CallstackEditor::draw_callstack_entry(CallStackItem &csi)
{
    if (!csi.src_address)
    {
        return;
    }

    ImGui::TableNextColumn();
    ImGui::Text("$%04X", csi.src_address);
    ImGui::TableNextColumn();
    ImGui::Text("%s", BootstrapIcons_arrow_right);
    ImGui::TableNextColumn();
    ImGui::Text("$%04X", csi.dst_address);
    ImGui::TableNextColumn();
    auto symbol = _session->symbols().get_symbol(csi.dst_address);
    if (symbol.override)
    {
        ImGui::Text("(%s)", symbol.symbol.c_str());
    }
}
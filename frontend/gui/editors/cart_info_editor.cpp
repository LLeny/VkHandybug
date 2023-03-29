#include "cart_info_editor.h"
#include "imgui.h"
#include "session.h"

CartInfoEditor::CartInfoEditor()
{
}

CartInfoEditor::~CartInfoEditor()
{
}

void CartInfoEditor::render()
{
    if (!enabled())
    {
        return;
    }

    auto system = _session->system();

    if (ImGui::BeginTable("cart_info", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableNextColumn();
        ImGui::Text("        [NAME]");
        ImGui::TableNextColumn();
        ImGui::Text("%s", system->CartGetName());

        ImGui::TableNextColumn();
        ImGui::Text("[MANUFACTURER]");
        ImGui::TableNextColumn();
        ImGui::Text("%s", system->CartGetManufacturer());

        ImGui::TableNextColumn();
        ImGui::Text("    [ROTATION]");
        ImGui::TableNextColumn();
        ImGui::Text("%d", system->CartGetRotate());

        ImGui::TableNextColumn();
        ImGui::Text("        [SIZE]");
        ImGui::TableNextColumn();
        ImGui::Text("%d", system->CartSize());

        ImGui::EndTable();
    }
}

#include "states_editor.h"
#include "imgui.h"
#include "bootstrap-icons.h"
#include <fmt/core.h>

StatesEditor::StatesEditor()
{
}

StatesEditor::~StatesEditor()
{
}

void StatesEditor::render()
{
    if (!enabled())
    {
        return;
    }

    render_header();

    if (ImGui::BeginTable("##statesitems", 3, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Del");
        ImGui::TableSetupColumn("Ld");
        ImGui::TableSetupColumn("Label");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (std::vector<State>::reverse_iterator rit = _session->states_manager().states().rbegin(); rit != _session->states_manager().states().rend(); ++rit)
        {
            render_state_item(*rit);
        }

        ImGui::EndTable();
    }
}

void StatesEditor::render_state_item(State &state)
{
    std::string filename = state.file.filename().generic_string();
    std::string name = filename.substr(0, filename.find_last_of("."));

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(15);
    std::string l1 = fmt::format("{}##sd{}", BootstrapIcons_trash, name.c_str());
    if (ImGui::Button(l1.c_str()))
    {
        delete_state(state);
    }

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(15);
    std::string l2 = fmt::format("{}##ld{}", BootstrapIcons_folder2_open, name.c_str());
    ImGui::BeginDisabled(is_read_only());
    if (ImGui::Button(l2.c_str()))
    {
        load_state(state);
    }
    ImGui::EndDisabled();

    ImGui::TableNextColumn();
    ImGui::Text("%s", name.c_str());
}

void StatesEditor::render_header()
{
    ImGui::BeginDisabled(is_read_only());
    if (ImGui::Button("Snap state"))
    {
        snap_state();
    }
    ImGui::EndDisabled();
}

void StatesEditor::snap_state()
{
    _session->states_manager().snap_state();
}

void StatesEditor::load_state(State &state)
{
    _session->states_manager().load_state(state);
}

void StatesEditor::delete_state(State &state)
{
    _session->states_manager().delete_state(state);
}

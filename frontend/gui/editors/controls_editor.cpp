#include "controls_editor.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include "vk_renderer.h"
#include "app.h"

ControlsEditor::ControlsEditor()
{
}

ControlsEditor::~ControlsEditor()
{
}

void ControlsEditor::render()
{
    if (!enabled())
    {
        return;
    }

    if (ImGui::BeginTable("##symbolsitems", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("Lynx");
        ImGui::TableSetupColumn("Key");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (int i = 0; i < LynxButtons_Max; ++i)
        {
            render_button_mapping((LynxButtons)i);
        }

        ImGui::EndTable();
    }
}

void ControlsEditor::render_button_mapping(LynxButtons btn)
{
    int glfw_key = get_mapping(btn);
    float font_size = ImGui::GetFontSize();

    ImGui::TableNextColumn();
    ImGui::Text("%s", get_description(btn).c_str());

    ImGui::TableNextColumn();
    std::string label = fmt::format("{}##btn{}", get_description(glfw_key).c_str(), (int)btn);
    std::string labelpop = fmt::format("##pop{}", (int)btn);
    if (ImGui::Button(label.c_str(), {5 * font_size, 0}))
    {
        _selected_key = glfw_key;
        ImGui::OpenPopup(labelpop.c_str());
    }

    bool open = true;

    ImGui::SetNextWindowSize({15 * font_size, 8 * font_size});
    if (ImGui::BeginPopupModal(labelpop.c_str(), &open))
    {
        int pressed = _session->_app->get_pressed_key();
        if (pressed > 0)
        {
            _selected_key = pressed;
        }

        auto windowWidth = ImGui::GetWindowSize().x;

        const char *header = "Press the desired key";
        auto header_width = ImGui::CalcTextSize(header).x;

        ImGui::SetCursorPosX((windowWidth - header_width) * 0.5f);
        ImGui::Text("%s", header);

        auto desc = get_description(_selected_key);
        auto desc_width = ImGui::CalcTextSize(desc.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - desc_width) * 0.5f);
        ImGui::Text("%s", desc.c_str());

        if (ImGui::Button("OK"))
        {
            set_mapping(_selected_key, btn);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

int ControlsEditor::get_mapping(LynxButtons btn)
{
    auto &mapping = _session->buttons_mapping();

    auto found = std::find_if(mapping.begin(), mapping.end(), [&btn](const std::unordered_map<int, LynxButtons>::value_type &vt) { return vt.second == btn; });

    if (found == mapping.end())
    {
        return -1;
    }

    return found->first;
}

void ControlsEditor::set_mapping(int glfw_key, LynxButtons btn)
{
    auto &mapping = _session->buttons_mapping();
    std::erase_if(mapping, [&btn](const std::unordered_map<int, LynxButtons>::value_type &vt) { return vt.second == btn; });
    mapping[glfw_key] = btn;
}

const std::string ControlsEditor::get_description(LynxButtons btn)
{
    return _lynx_names[btn];
}

const std::string ControlsEditor::get_description(int glfw_key)
{
    if (!_key_names.contains(glfw_key))
    {
        return "Error";
    }

    return _key_names.at(glfw_key);
}
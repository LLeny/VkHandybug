#include "global.h"
#include "editor.h"
#include "imgui.h"
#include "imgui_internal.h"

bool imgui_autocomplete_input(std::string label, char *buffer, size_t buffer_size, std::vector<std::string> &dictionary, ImGuiInputTextFlags flags)
{
    bool is_input_text_enter_pressed = ImGui::InputText(label.c_str(), buffer, buffer_size, ImGuiInputTextFlags_EnterReturnsTrue | flags);

    if (dictionary.size() <= 0)
    {
        return is_input_text_enter_pressed;
    }

    const bool is_input_text_active = ImGui::IsItemActive();
    const bool is_input_text_activated = ImGui::IsItemActivated();

    if (is_input_text_activated)
    {
        ImGui::OpenPopup("##popup");
    }

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
    ImGui::SetNextWindowSize({ImGui::GetItemRectSize().x * 1.5f, ImGui::GetTextLineHeight() * 8});
    if (ImGui::BeginPopup("##popup", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_ChildWindow))
    {
        for (auto &word : dictionary)
        {
            if (strstr(word.c_str(), buffer) == NULL)
            {
                continue;
            }

            if (ImGui::Selectable(word.c_str()))
            {
                ImGui::ClearActiveID();
                strcpy(buffer, word.c_str());
            }
        }

        if (is_input_text_enter_pressed || (!is_input_text_active && !ImGui::IsWindowFocused()))
        {
            ImGui::CloseCurrentPopup();
            is_input_text_enter_pressed = true;
        }

        ImGui::EndPopup();
    }

    return is_input_text_enter_pressed;
}
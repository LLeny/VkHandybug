#include "session_control_editor.h"
#include "imgui.h"
#include "session.h"
#include "bootstrap-icons.h"
#include <fmt/core.h>

SessionControlEditor::SessionControlEditor()
{
}

SessionControlEditor::~SessionControlEditor()
{
}

void SessionControlEditor::render()
{
    float font_size = ImGui::GetFontSize();
    float buttonHeight = font_size * 1.8;
    float buttonWidth = font_size * 3.5;

    if (ImGui::BeginTable("#controltable", 5, ImGuiTableFlags_NoBordersInBody))
    {
        ImGui::TableNextColumn();
        if (_session->status() == SessionStatus_Running)
        {
            if (create_button(BootstrapIcons_pause, {buttonWidth, buttonHeight}, "Pause - F6") || (ImGui::IsKeyPressed(ImGuiKey_F6) && _session->is_active()))
            {
                _session->set_status(SessionStatus_Break);
            }
        }
        else
        {
            if (create_button(BootstrapIcons_play, {buttonWidth, buttonHeight}, "Run - F5") || (ImGui::IsKeyPressed(ImGuiKey_F5) && _session->is_active()))
            {
                _session->set_status(SessionStatus_Running);
            }
        }
        ImGui::TableNextColumn();
        if (create_button(BootstrapIcons_skip_end, {buttonWidth, buttonHeight}, "Step - F10") || (ImGui::IsKeyPressed(ImGuiKey_F10) && _session->is_active()))
        {
            _session->set_status(SessionStatus_Step);
        }

        ImGui::TableNextColumn();
        if (create_button(BootstrapIcons_reply, {buttonWidth, buttonHeight}, "Step Over - F11") || (ImGui::IsKeyPressed(ImGuiKey_F11) && _session->is_active()))
        {
            _session->set_status(SessionStatus_Step_Over);
        }

        ImGui::TableNextColumn();
        if (create_button(BootstrapIcons_arrow_right, {buttonWidth, buttonHeight}, "Step Out - F12") || (ImGui::IsKeyPressed(ImGuiKey_F12) && _session->is_active()))
        {
            _session->set_status(SessionStatus_Step_Out);
        }

        ImGui::TableNextColumn();
        if (create_button(BootstrapIcons_arrow_repeat, {buttonWidth, buttonHeight}, "Reset - F8") || (ImGui::IsKeyPressed(ImGuiKey_F8) && _session->is_active()))
        {
            _session->system()->Reset();
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (_session->is_audio_enabled())
        {
            if (create_button(BootstrapIcons_volume_up, {buttonWidth, buttonHeight}, "Mute"))
            {
                _session->set_audio(false);
            }
        }
        else
        {
            if (create_button(BootstrapIcons_volume_mute, {buttonWidth, buttonHeight}, "Unmute"))
            {
                _session->set_audio(true);
            }
        }

        ImGui::TableNextColumn();
        if (_session->is_comlynx_enabled())
        {
            if (create_button(BootstrapIcons_wifi, {buttonWidth, buttonHeight}, "Disconnect ComLynx"))
            {
                _session->disable_comlynx();
            }
        }
        else
        {
            if (create_button(BootstrapIcons_wifi_off, {buttonWidth, buttonHeight}, "Connect ComLynx"))
            {
                _session->enable_comlynx();
            }
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (create_button("Rld", {buttonWidth, buttonHeight}, "Reload cart from disk"))
        {
            _session->set_status(SessionStatus_Break);
            _session->system()->ReloadCart();
            _session->reload_symbols();
        }

        ImGui::EndTable();
    }

    ImGui::Text("Version");
    ImGui::SameLine();

    int version = _session->get_lynx_version();
    if (ImGui::RadioButton("Lynx I", version == LynxVersion_1))
    {
        _session->set_lynx_version(LynxVersion_1);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Lynx II", version == LynxVersion_2))
    {
        _session->set_lynx_version(LynxVersion_2);
    }
}

bool SessionControlEditor::create_button(std::string text, ImVec2 size, std::string tooltip)
{
    bool ret = ImGui::Button(text.c_str(), size);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", tooltip.c_str());
    }
    return ret;
}

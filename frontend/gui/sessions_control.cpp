#include "sessions_control.h"
#include "app.h"
#include "session_gui.h"

SessionsControl::SessionsControl()
{
}

SessionsControl::~SessionsControl()
{
}

void SessionsControl::set_app(std::shared_ptr<App> app)
{
    _app = app;
}

bool SessionsControl::render()
{
    bool open = true;

    float buttonHeight = 20;
    float buttonWidth = 50;

    if (ImGui::Begin("Sessions control", &open))
    {
        if (ImGui::BeginTable("#sessionscontroltable", 6, ImGuiTableFlags_NoBordersInBody))
        {
            ImGui::TableNextColumn();
            if (create_button(BootstrapIcons_pause, {buttonWidth, buttonHeight}, "Pause All"))
            {
                for (auto session : _app->sessions())
                {
                    session->set_status(SessionStatus_Break);
                }
            }

            ImGui::TableNextColumn();
            if (create_button(BootstrapIcons_play, {buttonWidth, buttonHeight}, "Run All"))
            {
                for (auto session : _app->sessions())
                {
                    session->set_status(SessionStatus_Running);
                }
            }

            ImGui::TableNextColumn();
            if (create_button(BootstrapIcons_skip_end, {buttonWidth, buttonHeight}, "Step All"))
            {
                for (auto session : _app->sessions())
                {
                    session->set_status(SessionStatus_Step);
                }
            }

            ImGui::TableNextColumn();
            if (create_button(BootstrapIcons_reply, {buttonWidth, buttonHeight}, "Step Over All"))
            {
                for (auto session : _app->sessions())
                {
                    session->set_status(SessionStatus_Step_Over);
                }
            }

            ImGui::TableNextColumn();
            if (create_button(BootstrapIcons_arrow_right, {buttonWidth, buttonHeight}, "Step Out All"))
            {
                for (auto session : _app->sessions())
                {
                    session->set_status(SessionStatus_Step_Out);
                }
            }

            ImGui::TableNextColumn();
            if (create_button(BootstrapIcons_arrow_repeat, {buttonWidth, buttonHeight}, "Reset All"))
            {
                for (auto session : _app->sessions())
                {
                    session->system()->Reset();
                }
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (create_button("Rld", {buttonWidth, buttonHeight}, "Reload all carts from disk"))
            {
                for (auto session : _app->sessions())
                {
                    session->set_status(SessionStatus_Break);
                    session->system()->ReloadCart();
                    session->reload_symbols();
                }
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();

    return open;
}

bool SessionsControl::create_button(std::string text, ImVec2 size, std::string tooltip)
{
    bool ret = ImGui::Button(text.c_str(), size);
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", tooltip.c_str());
    }
    return ret;
}

#include "global.h"
#include "gui.h"
#include "imgui_internal.h"
#include "imgui.h"
#include "log.h"
#include "console.h"

GUI::GUI()
{
}

GUI::~GUI()
{
}

void GUI::initialize()
{
    _comlynx = std::make_shared<ComLynxVisualizer>();
}

Menu &GUI::menu()
{
    return _menu;
}

std::vector<std::shared_ptr<SessionGUI>> GUI::sessions()
{
    return _sessions;
}

void GUI::render(ImGuiID dockspace_id)
{
    _menu.render();

    if (ImGui::DockBuilderGetNode(dockspace_id) == NULL)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);                            // Clear out existing layout
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
    }

    for (auto session : _sessions)
    {
        auto sessionid = session->id();

        if (sessionid.length() <= 0)
        {
            continue;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(sessionid.c_str(), NULL);
        ImGui::PopStyleVar();

        session->build_dock();

        ImGui::End();

        session->render();
    }

    if (_comlynx_visible)
    {
        _comlynx_visible = _comlynx->render();
    }

    if (_console_visible)
    {
        Console::get_instance().render();
    }
}

std::shared_ptr<ComLynxVisualizer> GUI::comlynx_visualizer()
{
    return _comlynx;
}

void GUI::switch_comlynx_visualizer()
{
    _comlynx_visible = !_comlynx_visible;
}

void GUI::switch_console_visibility()
{
    _console_visible = !_console_visible;
}

std::shared_ptr<SessionGUI> GUI::register_session(std::shared_ptr<Session> session)
{
    auto gui = std::make_shared<SessionGUI>();

    gui->initialize(session);

    _sessions.push_back(gui);

    return gui;
}

void GUI::unregister_session(std::string sessionid)
{
    std::erase_if(_sessions, [sessionid](const std::shared_ptr<SessionGUI> a) { return a->id() == sessionid; });
}

std::shared_ptr<SessionGUI> GUI::get_session(std::string identifier)
{
    auto found = std::find_if(_sessions.begin(), _sessions.end(), [identifier](const std::shared_ptr<SessionGUI> a) { return a->id() == identifier; });

    if (found == _sessions.end())
    {
        return {};
    }

    return *found;
}
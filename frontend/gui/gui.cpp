#include "global.h"
#include "gui.h"
#include "imgui.h"
#include "log.h"
#include "bootstrap-icons.h"

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

void GUI::render()
{
    _menu.render();

    for (auto session : _sessions)
    {
        session->render();
    }

    if (_comlynx_visible)
    {
        _comlynx_visible = _comlynx->render();
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
#pragma once
#include "global.h"
#include "menu.h"
#include "sessions_control.h"
#include "session.h"
#include "session_gui.h"
#include "comlynx_visualizer.h"
#include "imgui.h"

class Config;
class Console;

class GUI
{
    friend Config;

  public:
    GUI();
    ~GUI();

    void initialize();

    std::shared_ptr<SessionGUI> register_session(std::shared_ptr<Session> session);
    void unregister_session(std::string sessionid);
    void render(ImGuiID dockspace_id);
    Menu &menu();
    std::vector<std::shared_ptr<SessionGUI>> sessions();
    std::shared_ptr<SessionGUI> get_session(std::string identifier);
    void switch_comlynx_visualizer();
    void switch_console_visibility();
    void switch_sessionscontrol_visibility();
    std::shared_ptr<ComLynxVisualizer> comlynx_visualizer();
    SessionsControl &sessions_control();

  private:
    bool _comlynx_visible;
    bool _console_visible;
    bool _sessionscontrol_visible;
    std::vector<std::shared_ptr<SessionGUI>> _sessions{};

    Menu _menu{};
    SessionsControl _sessionscontrol{};
    std::shared_ptr<ComLynxVisualizer> _comlynx;
};
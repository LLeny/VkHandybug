#pragma once
#include "global.h"
#include "menu.h"
#include "session.h"
#include "session_gui.h"
#include "comlynx_visualizer.h"
#include "imgui.h"

class Config;

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
    std::shared_ptr<ComLynxVisualizer> comlynx_visualizer();

  private:
    bool _comlynx_visible;
    std::vector<std::shared_ptr<SessionGUI>> _sessions{};

    Menu _menu{};
    std::shared_ptr<ComLynxVisualizer> _comlynx;
};
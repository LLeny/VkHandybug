#pragma once

#include "global.h"
#include "menu.h"
#include "gui.h"
#include "session.h"
#include "sound.h"
#include "comlynx_hub.h"
#include "vk_renderer.h"

class App : public std::enable_shared_from_this<App>
{
  public:
    App();
    ~App();

    void initialize();
    void close();
    void ask_to_close();
    void main_loop();

    std::shared_ptr<GUI> gui();
    ComLynxHub &comlynx_hub();

    int get_pressed_key();

    std::vector<std::shared_ptr<Session>> sessions();
    std::vector<std::string> &recent_sessions();
    void close_session(std::string session_identifier);
    void set_active_session(std::string session_identifier);
    bool is_active_session(std::string session_identifier);

    ImVec2 get_dimensions();
    ImVec2 get_position();
    void set_dimensions(ImVec2 dim);
    void set_position(ImVec2 pos);

  private:
    bool volatile _closing = false;

    std::string _active_session_identifier{};

    ComLynxHub _comlynx_hub{};
    std::shared_ptr<GUI> _gui;
    std::shared_ptr<VulkanRenderer> _renderer;

    std::thread _execute_thread;
    std::vector<std::shared_ptr<Session>> _sessions{};

    std::vector<std::string> _recent_sessions{};

    std::array<int, GLFW_KEY_LAST + 1> _key_statuses{};

    void execute();
    void render();
    void process_key_event(int glfw_key, int mods, bool glfw_action);
    void open_file(std::string file);
    void new_session(std::filesystem::path file);
};
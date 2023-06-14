#pragma once
#include "global.h"
#include "settings.h"
#include "bootstrap-icons.h"
#include "imgui.h"

class App;

enum MenuItemType
{
    MENUITEM_TYPE_UNKNOWN = 0,
    MENUITEM_TYPE_SUB_MENU = 1,
    MENUITEM_TYPE_CALLBACK = 2,
    MENUITEM_TYPE_SEPARATOR = 3,
};

struct MenuItem
{
    std::string session_dentifier;
    std::string label{};
    std::string shortcut{};
    MenuItemType type = MENUITEM_TYPE_UNKNOWN;
    std::function<void(void)> callback{};
    ImGuiKeyChord keyCord{};
    std::vector<MenuItem> subitems{};
    void *user_data{};
};

class Menu
{
  public:
    Menu();
    ~Menu();

    void render();
    void register_file_open_callback(std::function<void(std::string file)> callback);
    void set_app(std::shared_ptr<App> app);
    Settings &settings();

  private:
    std::shared_ptr<App> _app;
    std::vector<MenuItem> _items{};
    std::vector<MenuItem> _session_items{};

    Settings _settings{};
    bool _settings_popup_open = false;
    const char *_settings_popup_id{};

    std::function<void(std::string file)> _file_open_callback;

    void initialize();
    void process_keys();
    void process_keys(MenuItem &item);
    void render(MenuItem &item);
    void render_popups();
    void update_items();

    void add_bool_callback_item(std::string session_identifier, std::vector<MenuItem> &list, bool *selected, std::string header, std::string shortcut, ImGuiKeyChord keycord);
    void call_fileopen_callback(std::string file);

    void load_symbols(std::string sessionid, std::string path);
};
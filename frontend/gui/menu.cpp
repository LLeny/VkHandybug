#include "menu.h"
#include "imgui.h"
#include "ImGuiFileDialog.h"
#include "config.h"
#include "app.h"
#include "session.h"
#include "bootstrap-icons.h"
#include <fmt/core.h>
#include "imgui_internal.h"

Menu::Menu()
{
    initialize();
}

Menu::~Menu()
{
}

void Menu::render()
{
    update_items();

    render_popups();

    if (ImGui::BeginMainMenuBar())
    {
        for (auto &item : _items)
        {
            render(item);
        }
        ImGui::EndMainMenuBar();
    }

    process_keys();
}

void Menu::render(MenuItem &item)
{
    switch (item.type)
    {
    case MENUITEM_TYPE_SUB_MENU:
        if (ImGui::BeginMenu(item.label.c_str()))
        {
            for (auto &subitem : item.subitems)
            {
                render(subitem);
            }
            ImGui::EndMenu();
        }
        break;
    case MENUITEM_TYPE_SEPARATOR:
        ImGui::Separator();
        break;
    case MENUITEM_TYPE_CALLBACK: {
        if (ImGui::MenuItem(item.label.c_str(), item.shortcut.c_str()))
        {
            auto future = std::async(std::launch::async, [item]() { item.callback(); });
        }
    }
    break;
    default:
        break;
    }
}

void Menu::set_app(std::shared_ptr<App> app)
{
    _app = std::move(app);
}

void Menu::register_file_open_callback(std::function<void(std::string file)> callback)
{
    _file_open_callback = callback;
}

void Menu::initialize()
{
    std::vector<MenuItem> mainitems{};

    _settings_popup_id = BootstrapIcons_gear " SETTINGS";

    _settings.initialize(_app);

    mainitems.push_back({"", "New Session...", "Ctrl+N", MENUITEM_TYPE_CALLBACK, [&]() {
                             ImGuiFileDialog::Instance()->OpenDialog("ChooseCartridge", "Cartridge", ".lnx,.o", Config::getInstance().store().last_rom_folder, 1, nullptr, ImGuiFileDialogFlags_Modal);
                         },
                         ImGuiMod_Ctrl | ImGuiKey_N});
    mainitems.push_back({"", "Settings...", "", MENUITEM_TYPE_CALLBACK, [&]() {
                             _settings_popup_open = true;
                         }});
    mainitems.push_back({"", "", "", MENUITEM_TYPE_SEPARATOR});
    mainitems.push_back({"", "Quit", "Ctrl+X", MENUITEM_TYPE_CALLBACK, [&]() { _app->ask_to_close(); }, ImGuiMod_Ctrl | ImGuiKey_X});

    _items.push_back({"", "MAIN", "", MENUITEM_TYPE_SUB_MENU, nullptr, 0, mainitems});
    _items.push_back({"", "SESSIONS", "", MENUITEM_TYPE_SUB_MENU, nullptr, 0, _session_items});

    std::vector<MenuItem> toolsitems{};

    toolsitems.push_back({"", "ComLynx", "", MENUITEM_TYPE_CALLBACK, [&]() {
                              _app->gui()->switch_comlynx_visualizer();
                          }});

    _items.push_back({"", "TOOLS", "", MENUITEM_TYPE_SUB_MENU, nullptr, 0, toolsitems});
}

Settings &Menu::settings()
{
    return _settings;
}

void Menu::render_popups()
{
    // Settings
    if (_settings_popup_open)
    {
        ImGui::OpenPopup(_settings_popup_id);

        ImGui::SetNextWindowSize({400, 120});
        if (ImGui::BeginPopupModal(_settings_popup_id))
        {
            if (!_settings.render())
            {
                _settings_popup_open = false;
                _settings.set_update_pending(true);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    // Cartridge chooser
    if (ImGuiFileDialog::Instance()->Display("ChooseCartridge", ImGuiWindowFlags_None, {600, 300}))
    {
        if (ImGuiFileDialog::Instance()->IsOk() && _file_open_callback)
        {
#ifdef _MSC_VER
            auto sep = std::wstring(&std::filesystem::path::preferred_separator);
            Config::getInstance().store().last_rom_folder = ImGuiFileDialog::Instance()->GetCurrentPath() + std::string(sep.begin(), sep.end());
#else
            Config::getInstance().store().last_rom_folder = ImGuiFileDialog::Instance()->GetCurrentPath() + std::filesystem::path::preferred_separator;
#endif
            call_fileopen_callback(ImGuiFileDialog::Instance()->GetFilePathName());
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

void Menu::process_keys()
{
    for (auto &item : _items)
    {
        process_keys(item);
    }
}

void Menu::process_keys(MenuItem &item)
{
    if (item.type == MENUITEM_TYPE_SUB_MENU)
    {
        for (auto &subitem : item.subitems)
        {
            process_keys(subitem);
        }
        return;
    }

    if (item.type != MENUITEM_TYPE_CALLBACK || 0 == item.keyCord)
    {
        return;
    }

    if (item.keyCord & ImGuiMod_Ctrl)
    {
        if (!ImGui::GetIO().KeyCtrl)
        {
            return;
        }
    }

    ImGuiKey key = (ImGuiKey)(item.keyCord & ~ImGuiMod_Ctrl);

    if (ImGui::IsKeyPressed(key))
    {
        if (!item.session_dentifier.empty() && !_app->is_active_session(item.session_dentifier))
        {
            return;
        }

        auto future = std::async(std::launch::async, [item]() { item.callback(); });
    }
}

void Menu::call_fileopen_callback(std::string file)
{
    _file_open_callback(file);
}

void Menu::update_items()
{
    // Recent sessions
    auto main = std::find_if(_items.begin(), _items.end(), [](MenuItem &item) { return item.label == "MAIN"; });

    while (main->subitems.back().label != "Quit")
    {
        main->subitems.pop_back();
    }

    auto recents = _app->recent_sessions();
    if (recents.size() > 0)
    {
        main->subitems.push_back({"", "", "", MENUITEM_TYPE_SEPARATOR});

        for (auto recent : recents)
        {
            main->subitems.push_back({"", recent, "", MENUITEM_TYPE_CALLBACK, [this, recent]() {
                                          call_fileopen_callback(recent);
                                      }});
        }
    }

    // Sessions
    auto sess = std::find_if(_items.begin(), _items.end(), [](MenuItem &item) { return item.label == "SESSIONS"; });

    sess->subitems.clear();

    for (auto session : _app->sessions())
    {
        std::vector<MenuItem> subs{};

        auto sessionid = session->identifier();

        auto guisession = _app->gui()->get_session(sessionid);

        add_bool_callback_item(sessionid, subs, &guisession->_breakpoints_open, "breakpoints", "Ctrl+B", ImGuiMod_Ctrl | ImGuiKey_B);
        add_bool_callback_item(sessionid, subs, &guisession->_callstack_open, "callstack", "Ctrl+A", ImGuiMod_Ctrl | ImGuiKey_A);
        add_bool_callback_item(sessionid, subs, &guisession->_cart_info_open, "cart information", "Ctrl+I", ImGuiMod_Ctrl | ImGuiKey_I);
        add_bool_callback_item(sessionid, subs, &guisession->_controls_open, "controls", "Ctrl+O", ImGuiMod_Ctrl | ImGuiKey_O);
        add_bool_callback_item(sessionid, subs, &guisession->_cpu_open, "CPU", "Ctrl+C", ImGuiMod_Ctrl | ImGuiKey_C);
        add_bool_callback_item(sessionid, subs, &guisession->_mikie_open, "mikey", "Ctrl+K", ImGuiMod_Ctrl | ImGuiKey_K);
        add_bool_callback_item(sessionid, subs, &guisession->_palette_open, "palette", "Ctrl+P", ImGuiMod_Ctrl | ImGuiKey_P);
        add_bool_callback_item(sessionid, subs, &guisession->_states_manager_open, "states", "Ctrl+T", ImGuiMod_Ctrl | ImGuiKey_T);
        add_bool_callback_item(sessionid, subs, &guisession->_suzy_open, "suzy", "Ctrl+U", ImGuiMod_Ctrl | ImGuiKey_U);
        add_bool_callback_item(sessionid, subs, &guisession->_symbols_open, "symbols", "Ctrl+S", ImGuiMod_Ctrl | ImGuiKey_S);
        add_bool_callback_item(sessionid, subs, &guisession->_watch_open, "watch", "Ctrl+W", ImGuiMod_Ctrl | ImGuiKey_W);

        subs.push_back({sessionid, "New memory editor", "Ctrl+M", MENUITEM_TYPE_CALLBACK, [guisession]() {
                            guisession->add_memory_editor(-1);
                        },
                        ImGuiMod_Ctrl | ImGuiKey_M});

        subs.push_back({sessionid, "New dissasembly view", "Ctrl+D", MENUITEM_TYPE_CALLBACK, [guisession]() {
                            guisession->add_disasm_editor(-1);
                        },
                        ImGuiMod_Ctrl | ImGuiKey_D});

        subs.push_back({sessionid, "Close session", "", MENUITEM_TYPE_CALLBACK, [&, sessionid]() {
                            LOG(LOG_DEBUG) << "Menu - close_session " << sessionid;
                            _app->close_session(sessionid);
                        },
                        0});

        sess->subitems.push_back({sessionid, sessionid, "", MENUITEM_TYPE_SUB_MENU, nullptr, 0, subs});
    }
}

void Menu::add_bool_callback_item(std::string session_identifier, std::vector<MenuItem> &list, bool *selected, std::string header, std::string shortcut, ImGuiKeyChord keycord)
{
    list.push_back({session_identifier, *selected ? fmt::format("Hide {}", header) : fmt::format("Show {}", header), shortcut.c_str(), MENUITEM_TYPE_CALLBACK, [selected]() {
                        *selected = !*selected;
                    },
                    keycord});
}
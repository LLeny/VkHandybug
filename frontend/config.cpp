#include "config.h"
#include "cfgpath.h"
#include "log.h"
#include "app.h"
#include "imgui.h"
#include "bootstrap-icons_lib.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "session_gui.h"
#include "memory_editor.h"
#include "imgui_memory_editor.h"
#include "disasm_editor.h"
#include "breakpoints_editor.h"
#include "watch_editor.h"

Config::Config()
{
    auto imgui_ini_str = imgui_ini_file().generic_string();
    strncpy(_imgui_ini_file, imgui_ini_str.c_str(), MIN(imgui_ini_str.length(), sizeof(_imgui_ini_file)));
}

void Config::load(App *app)
{
    std::string filename = config_file().generic_string();
    try
    {
        std::ifstream is(filename);
        cereal::JSONInputArchive archive(is);

        archive(CEREAL_NVP(_store));
    }
    catch (std::exception &e)
    {
        LOG(LOGLEVEL_ERROR) << "Config - Couldn't load config file " << filename;
    }

    initialize();
    load_recents(app);

    app->set_dimensions({(float)_store.main_window_width, (float)_store.main_window_height});
    app->set_position({(float)_store.main_window_x_pos, (float)_store.main_window_y_pos});

    app->gui()->_comlynx_visible = _store.comlynx_visisble;
    app->gui()->_console_visible = _store.console_visible;
    app->gui()->_sessionscontrol_visible = _store.sessionscontrol_visible;
    Console::get_instance().set_log_level(_store.log_level);
}

void Config::save_recents(App *app)
{
    _store.recent_sessions.clear();

    for (auto recent : app->recent_sessions())
    {
        _store.recent_sessions.push_back(recent);
    }
}

void Config::load_recents(App *app)
{
    app->recent_sessions().clear();

    for (auto recent : _store.recent_sessions)
    {
        app->recent_sessions().push_back(recent);
    }
}

void Config::initialize()
{
    apply_theme();
    // apply_font(); // https://github.com/ocornut/imgui/pull/3761
}

void Config::save(App *app)
{
    save_recents(app);

    auto dim = app->get_dimensions();
    auto pos = app->get_position();

    _store.main_window_x_pos = pos.x;
    _store.main_window_y_pos = pos.y;
    _store.main_window_width = dim.x;
    _store.main_window_height = dim.y;

    _store.comlynx_visisble = app->gui()->_comlynx_visible;
    _store.console_visible = app->gui()->_console_visible;
    _store.sessionscontrol_visible = app->gui()->_sessionscontrol_visible;
    _store.log_level = Console::get_instance().get_log_level();

    std::string filename = config_file().generic_string();

    try
    {
        std::ofstream os(filename);
        cereal::JSONOutputArchive archive(os);

        archive(CEREAL_NVP(_store));
    }
    catch (std::exception &e)
    {
        LOG(LOGLEVEL_ERROR) << "Config - Couldn't save config file " << filename;
    }
}

void Config::save_sessions(std::vector<std::shared_ptr<SessionGUI>> sessions)
{
    for (auto &session : sessions)
    {
        std::vector<BreakpointConfigStore> breakpoints{};
        std::vector<WatchConfigStore> watches{};
        std::unordered_map<int, int> buttons_mapping{};

        for (auto &bp : session->_session->breakpoints())
        {
            breakpoints.push_back({bp.enabled, bp.address, bp.bank, bp.type, bp.script});
        }

        for (auto &w : session->_watch_editor.watches())
        {
            watches.push_back({w.id, w.label, w.type, w.bank, w.address});
        }

        for (auto btnmap : session->_session->buttons_mapping())
        {
            buttons_mapping[btnmap.first] = btnmap.second;
        }

        auto found = std::find_if(_store.sessions.begin(), _store.sessions.end(), [session](SessionConfigStore &s) { return s.id == session->id(); });

        if (found == _store.sessions.end())
        {
            SessionConfigStore store{};

            store.id = session->_session->identifier();
            store.cpu_visible = session->_cpu_open;
            store.watch_visible = session->_watch_open;
            store.palette_vsible = session->_palette_open;
            store.cart_info_visible = session->_cart_info_open;
            store.symbols_visible = session->_symbols_open;
            store.breakpoints_visible = session->_breakpoints_open;
            store.states_manager_visible = session->_states_manager_open;
            store.controls_visible = session->_controls_open;
            store.mikie_visible = session->_mikie_open;
            store.suzy_visible = session->_suzy_open;
            store.callstack_visible = session->_callstack_open;
            store.comlynx_connected = session->_session->is_comlynx_enabled();
            store.muted = !session->_session->is_audio_enabled();
            store.breakpoints = breakpoints;
            store.watches = watches;
            store.buttons_mapping = buttons_mapping;
            store.lynx_version = session->_session->get_lynx_version();

            _store.sessions.push_back(store);
        }
        else
        {
            found->cpu_visible = session->_cpu_open;
            found->watch_visible = session->_watch_open;
            found->palette_vsible = session->_palette_open;
            found->cart_info_visible = session->_cart_info_open;
            found->symbols_visible = session->_symbols_open;
            found->breakpoints_visible = session->_breakpoints_open;
            found->states_manager_visible = session->_states_manager_open;
            found->controls_visible = session->_controls_open;
            found->mikie_visible = session->_mikie_open;
            found->suzy_visible = session->_suzy_open;
            found->callstack_visible = session->_callstack_open;
            found->comlynx_connected = session->_session->is_comlynx_enabled();
            found->muted = !session->_session->is_audio_enabled();
            found->breakpoints = breakpoints;
            found->watches = watches;
            found->buttons_mapping = buttons_mapping;
            found->lynx_version = session->_session->get_lynx_version();
        }

        for (auto memedit : session->_mem_editors)
        {
            save_memory_editor(session->_session->identifier(), memedit.get());
        }

        for (auto disasmedit : session->_disasm_editors)
        {
            save_disasm_editor(session->_session->identifier(), disasmedit.get());
        }
    }
}

void Config::load_session(std::shared_ptr<SessionGUI> session)
{
    if (!session)
    {
        return;
    }

    auto found = std::find_if(_store.sessions.begin(), _store.sessions.end(), [session](SessionConfigStore &s) { return s.id == session->id(); });

    if (found == _store.sessions.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find session '" << session->id() << "'";
        return;
    }

    session->_cart_info_open = found->cart_info_visible;
    session->_cpu_open = found->cpu_visible;
    session->_palette_open = found->palette_vsible;
    session->_watch_open = found->watch_visible;
    session->_symbols_open = found->symbols_visible;
    session->_breakpoints_open = found->breakpoints_visible;
    session->_states_manager_open = found->states_manager_visible;
    session->_controls_open = found->controls_visible;
    session->_mikie_open = found->mikie_visible;
    session->_suzy_open = found->suzy_visible;
    session->_callstack_open = found->callstack_visible;
    if (found->comlynx_connected)
    {
        session->_session->enable_comlynx();
    }
    session->_session->set_audio(!found->muted);

    for (auto &mem_edit : found->mem_editors)
    {
        session->add_memory_editor(mem_edit.id);
    }

    for (auto &disasm_edit : found->disasm_editors)
    {
        session->add_disasm_editor(disasm_edit.id);
    }

    session->_session->breakpoints().clear();

    for (auto &bp : found->breakpoints)
    {
        session->_session->add_breakpoint(bp.address, bp.bank, bp.type, bp.script);
    }

    session->_watch_editor.watches().clear();

    for (auto &w : found->watches)
    {
        session->_watch_editor.watches().push_back({w.id, w.label, w.type, w.bank, w.address});
    }

    session->_session->buttons_mapping().clear();

    for (auto &m : found->buttons_mapping)
    {
        session->_session->buttons_mapping()[m.first] = (LynxButtons)m.second;
    }

    session->_session->set_lynx_version(found->lynx_version);
}

void Config::save_memory_editor(std::string sessionid, MemEditor *editor)
{
    auto found = std::find_if(_store.sessions.begin(), _store.sessions.end(), [sessionid](SessionConfigStore &s) { return s.id == sessionid; });

    if (found == _store.sessions.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find session '" << sessionid << "'";
        return;
    }

    int editorid = editor->id();

    auto memconfig = std::find_if(found->mem_editors.begin(), found->mem_editors.end(), [editorid](SessionMemEditorConfigStore &s) { return s.id == editorid; });

    if (memconfig != found->mem_editors.end())
    {
        memconfig->optAddrDigitsCount = editor->_memoryEditor.OptAddrDigitsCount;
        memconfig->optFooterExtraHeight = editor->_memoryEditor.OptFooterExtraHeight;
        memconfig->optGreyOutZeroes = editor->_memoryEditor.OptGreyOutZeroes;
        memconfig->optMidColsCount = editor->_memoryEditor.OptMidColsCount;
        memconfig->optShowAscii = editor->_memoryEditor.OptShowAscii;
        memconfig->optShowDataPreview = editor->_memoryEditor.OptShowDataPreview;
        memconfig->optShowHexII = editor->_memoryEditor.OptShowHexII;
        memconfig->optShowOptions = editor->_memoryEditor.OptShowOptions;
        memconfig->optUpperCaseHex = editor->_memoryEditor.OptUpperCaseHex;
        memconfig->optHighlightChanges = editor->_memoryEditor.OptHighlightChanges;
        memconfig->selected_bank = editor->_memBank;
    }
    else
    {
        SessionMemEditorConfigStore store{};
        store.id = editor->id();
        store.optAddrDigitsCount = editor->_memoryEditor.OptAddrDigitsCount;
        store.optFooterExtraHeight = editor->_memoryEditor.OptFooterExtraHeight;
        store.optGreyOutZeroes = editor->_memoryEditor.OptGreyOutZeroes;
        store.optMidColsCount = editor->_memoryEditor.OptMidColsCount;
        store.optShowAscii = editor->_memoryEditor.OptShowAscii;
        store.optShowDataPreview = editor->_memoryEditor.OptShowDataPreview;
        store.optShowHexII = editor->_memoryEditor.OptShowHexII;
        store.optShowOptions = editor->_memoryEditor.OptShowOptions;
        store.optUpperCaseHex = editor->_memoryEditor.OptUpperCaseHex;
        store.optHighlightChanges = editor->_memoryEditor.OptHighlightChanges;
        store.selected_bank = editor->_memBank;
        found->mem_editors.push_back(store);
    }
}

void Config::delete_memory_editor(std::string sessionid, MemEditor *editor)
{
    auto found = std::find_if(_store.sessions.begin(), _store.sessions.end(), [sessionid](SessionConfigStore &s) { return s.id == sessionid; });

    if (found == _store.sessions.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find session '" << sessionid << "'";
        return;
    }

    int editorid = editor->id();

    std::erase_if(found->mem_editors, [editorid](SessionMemEditorConfigStore &s) { return s.id == editorid; });
}

void Config::load_memory_editor(std::string sessionid, MemEditor *editor)
{
    auto found = std::find_if(_store.sessions.begin(), _store.sessions.end(), [sessionid](SessionConfigStore &s) { return s.id == sessionid; });

    if (found == _store.sessions.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find session '" << sessionid << "'";
        return;
    }

    int editorid = editor->id();

    auto memconfig = std::find_if(found->mem_editors.begin(), found->mem_editors.end(), [editorid](SessionMemEditorConfigStore &s) { return s.id == editorid; });

    if (memconfig == found->mem_editors.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find mem editor '" << editorid << "'";
        return;
    }

    editor->_memoryEditor.OptAddrDigitsCount = memconfig->optAddrDigitsCount;
    editor->_memoryEditor.OptFooterExtraHeight = memconfig->optFooterExtraHeight;
    editor->_memoryEditor.OptGreyOutZeroes = memconfig->optGreyOutZeroes;
    editor->_memoryEditor.OptMidColsCount = memconfig->optMidColsCount;
    editor->_memoryEditor.OptShowAscii = memconfig->optShowAscii;
    editor->_memoryEditor.OptShowDataPreview = memconfig->optShowDataPreview;
    editor->_memoryEditor.OptShowHexII = memconfig->optShowHexII;
    editor->_memoryEditor.OptShowOptions = memconfig->optShowOptions;
    editor->_memoryEditor.OptUpperCaseHex = memconfig->optUpperCaseHex;
    editor->_memoryEditor.OptHighlightChanges = memconfig->optHighlightChanges;
    editor->_memBank = (LynxMemBank)memconfig->selected_bank;
}

void Config::save_disasm_editor(std::string sessionid, DisasmEditor *editor)
{
    auto found = std::find_if(_store.sessions.begin(), _store.sessions.end(), [sessionid](SessionConfigStore &s) { return s.id == sessionid; });

    if (found == _store.sessions.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find session '" << sessionid << "'";
        return;
    }

    int editorid = editor->id();

    auto disasmconfig = std::find_if(found->disasm_editors.begin(), found->disasm_editors.end(), [editorid](SessionDisasmEditorConfigStore &s) { return s.id == editorid; });

    if (disasmconfig != found->disasm_editors.end())
    {
        disasmconfig->follow_pc = editor->_follow_pc;
        disasmconfig->show_labels = editor->_show_labels;
        disasmconfig->local_pc = editor->_local_pc;
    }
    else
    {
        SessionDisasmEditorConfigStore store{};
        store.id = editor->id();
        store.follow_pc = editor->_follow_pc;
        store.show_labels = editor->_show_labels;
        store.local_pc = editor->_local_pc;
        found->disasm_editors.push_back(store);
    }
}

void Config::delete_disasm_editor(std::string sessionid, DisasmEditor *editor)
{
    auto found = std::find_if(_store.sessions.begin(), _store.sessions.end(), [sessionid](SessionConfigStore &s) { return s.id == sessionid; });

    if (found == _store.sessions.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find session '" << sessionid << "'";
        return;
    }

    int editorid = editor->id();

    std::erase_if(found->disasm_editors, [editorid](SessionDisasmEditorConfigStore &s) { return s.id == editorid; });
}

void Config::load_disasm_editor(std::string sessionid, DisasmEditor *editor)
{
    auto found = std::find_if(_store.sessions.begin(), _store.sessions.end(), [sessionid](SessionConfigStore &s) { return s.id == sessionid; });

    if (found == _store.sessions.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find session '" << sessionid << "'";
        return;
    }

    int editorid = editor->id();

    auto disasmconfig = std::find_if(found->disasm_editors.begin(), found->disasm_editors.end(), [editorid](SessionDisasmEditorConfigStore &s) { return s.id == editorid; });

    if (disasmconfig == found->disasm_editors.end())
    {
        LOG(LOGLEVEL_INFO) << "Config - Couldn't find disasm editor '" << editorid << "'";
        return;
    }

    editor->_follow_pc = disasmconfig->follow_pc;
    editor->_show_labels = disasmconfig->show_labels;
    editor->_local_pc = disasmconfig->local_pc;
}

ConfigStore &Config::store()
{
    return _store;
}

std::filesystem::path Config::config_file()
{
    char cfgdir[512];
    get_user_config_folder(cfgdir, sizeof(cfgdir), APP_NAME);
    if (cfgdir[0] == 0)
    {
        LOG(LOGLEVEL_ERROR) << "Config - Unable to find home directory.";
        return std::filesystem::path{};
    }
    return std::filesystem::path(cfgdir) / "config";
}

char *Config::imgui_ini()
{
    return _imgui_ini_file;
}

std::filesystem::path Config::imgui_ini_file()
{
    char cfgdir[512];
    get_user_config_folder(cfgdir, sizeof(cfgdir), APP_NAME);
    if (cfgdir[0] == 0)
    {
        LOG(LOGLEVEL_ERROR) << "Config - Unable to find home directory.";
        return std::filesystem::path{};
    }
    return std::filesystem::path(cfgdir) / "imgui.ini";
}

void Config::apply_theme()
{
    if (_store.theme == "dark")
    {
        ImGui::StyleColorsDark();
    }
    else
    {
        ImGui::StyleColorsLight();
    }
}

void Config::apply_font(float scale)
{
    ImGuiIO &io = ImGui::GetIO();

    io.Fonts->Clear();

    int font_size = scale * 16;

    ImFontConfig cfg;
    cfg.MergeMode = true;
    cfg.GlyphOffset = {0.f, -4.f};
    cfg.GlyphMinAdvanceX = font_size;

    io.Fonts->AddFontFromFileTTF(_store.font.c_str(), font_size);
    ImFont *font = BootstrapIcons::Font::Load(io, font_size, &cfg);
    io.Fonts->Build();

    ImGui::GetStyle().ScaleAllSizes(2.0);
}
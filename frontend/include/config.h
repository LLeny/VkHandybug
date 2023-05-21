#pragma once

#include "global.h"
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include "imgui.h"
#include "breakpoints_editor.h"

class SessionGUI;
class MemEditor;
class DisasmEditor;
class App;

struct WatchConfigStore
{
    uint32_t id = 0;
    std::string label;
    ImGuiDataType type = ImGuiDataType_U8;
    LynxMemBank bank = LynxMemBank_RAM;
    uint16_t address = 0;

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(id));
        archive(CEREAL_NVP(label));
        archive(CEREAL_NVP(type));
        archive(CEREAL_NVP(bank));
        archive(CEREAL_NVP(address));
    }
};

struct BreakpointConfigStore
{
    bool enabled;
    uint16_t address;
    LynxMemBank bank;
    BreakPointType type;
    std::string script;

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(enabled));
        archive(CEREAL_NVP(address));
        archive(CEREAL_NVP(bank));
        archive(CEREAL_NVP(type));
        archive(CEREAL_NVP(script));
    }
};

struct SessionDisasmEditorConfigStore
{
    int id;
    bool follow_pc;
    bool show_labels;
    uint16_t local_pc;

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(id));
        archive(CEREAL_NVP(follow_pc));
        archive(CEREAL_NVP(show_labels));
        archive(CEREAL_NVP(local_pc));
    }
};

struct SessionMemEditorConfigStore
{
    int id;
    bool optShowOptions;
    bool optShowDataPreview;
    bool optShowHexII;
    bool optShowAscii;
    bool optGreyOutZeroes;
    bool optUpperCaseHex;
    int optMidColsCount;
    int optAddrDigitsCount;
    float optFooterExtraHeight;
    int selected_bank;

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(id));
        archive(CEREAL_NVP(optShowOptions));
        archive(CEREAL_NVP(optShowDataPreview));
        archive(CEREAL_NVP(optShowHexII));
        archive(CEREAL_NVP(optShowAscii));
        archive(CEREAL_NVP(optGreyOutZeroes));
        archive(CEREAL_NVP(optUpperCaseHex));
        archive(CEREAL_NVP(optMidColsCount));
        archive(CEREAL_NVP(optAddrDigitsCount));
        archive(CEREAL_NVP(optFooterExtraHeight));
        archive(CEREAL_NVP(selected_bank));
    }
};

struct SessionConfigStore
{
    std::string id;
    bool cpu_visible;
    bool watch_visible;
    bool palette_vsible;
    bool cart_info_visible;
    bool symbols_visible;
    bool breakpoints_visible;
    bool states_manager_visible;
    bool controls_visible;
    bool mikie_visible;
    bool suzy_visible;
    bool callstack_visible;
    bool muted;
    bool comlynx_connected;

    std::vector<SessionMemEditorConfigStore> mem_editors{};
    std::vector<SessionDisasmEditorConfigStore> disasm_editors{};
    std::vector<BreakpointConfigStore> breakpoints;
    std::vector<WatchConfigStore> watches;
    std::unordered_map<int, int> buttons_mapping{};

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(cart_info_visible));
        archive(CEREAL_NVP(cpu_visible));
        archive(CEREAL_NVP(id));
        archive(CEREAL_NVP(palette_vsible));
        archive(CEREAL_NVP(watch_visible));
        archive(CEREAL_NVP(symbols_visible));
        archive(CEREAL_NVP(breakpoints_visible));
        archive(CEREAL_NVP(states_manager_visible));
        archive(CEREAL_NVP(callstack_visible));
        archive(CEREAL_NVP(mikie_visible));
        archive(CEREAL_NVP(suzy_visible));
        archive(CEREAL_NVP(mem_editors));
        archive(CEREAL_NVP(disasm_editors));
        archive(CEREAL_NVP(breakpoints));
        archive(CEREAL_NVP(watches));
        archive(CEREAL_NVP(buttons_mapping));
        archive(CEREAL_NVP(muted));
        archive(CEREAL_NVP(comlynx_connected));
    }
};

struct ConfigStore
{
    std::string font = "fonts/Crisp.ttf";
    std::string theme = "dark";
    std::string last_rom_folder = ".";
    std::string lynx_rom_file = "./lynxboot.img";
    bool break_on_illegal_opcode = true;
    int main_window_x_pos = 10, main_window_y_pos = 10;
    int main_window_width = 800, main_window_height = 600;
    bool comlynx_visisble = false;
    bool console_visible = false;
    LOGLEVEL_ log_level = LOGLEVEL_WARN;
    std::vector<std::string> recent_sessions{};

    std::vector<SessionConfigStore> sessions{};

    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(CEREAL_NVP(font));
        archive(CEREAL_NVP(theme));
        archive(CEREAL_NVP(last_rom_folder));
        archive(CEREAL_NVP(lynx_rom_file));
        archive(CEREAL_NVP(break_on_illegal_opcode));
        archive(CEREAL_NVP(main_window_x_pos));
        archive(CEREAL_NVP(main_window_y_pos));
        archive(CEREAL_NVP(main_window_width));
        archive(CEREAL_NVP(main_window_height));
        archive(CEREAL_NVP(comlynx_visisble));
        archive(CEREAL_NVP(console_visible));
        archive(CEREAL_NVP(recent_sessions));
        archive(CEREAL_NVP(log_level));
        archive(CEREAL_NVP(sessions));
    }
};

class Config
{
  public:
    static Config &getInstance()
    {
        static Config instance;
        return instance;
    }
    Config(Config const &) = delete;
    void operator=(Config const &) = delete;

    ConfigStore &store();
    void load(App *app);
    void save(App *app);

    void apply_theme();
    void apply_font();
    void load_recents(App *app);
    void save_recents(App *app);
    void initialize();

    void save_sessions(std::vector<std::shared_ptr<SessionGUI>> sessions);
    void load_session(std::shared_ptr<SessionGUI> session);
    void load_memory_editor(std::string sessionid, MemEditor *editor);
    void save_memory_editor(std::string sessionid, MemEditor *editor);
    void delete_memory_editor(std::string sessionid, MemEditor *editor);
    void load_disasm_editor(std::string sessionid, DisasmEditor *editor);
    void save_disasm_editor(std::string sessionid, DisasmEditor *editor);
    void delete_disasm_editor(std::string sessionid, DisasmEditor *editor);

  private:
    Config(){};

    std::filesystem::path config_file();

    ConfigStore _store{};
};
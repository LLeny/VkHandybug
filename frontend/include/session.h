#pragma once

#include "global.h"
#include "system.h"
#include "imgui.h"
#include "symbols.h"
#include "states_manager.h"
#include <GLFW/glfw3.h>

#define CYCLE_TIME_NS (250)
#define RTS 0x60
#define JSR 0x20

class VulkanRenderer;
class App;
class Config;
class ControlsEditor;

enum BreakPointType
{
    BreakPointType_EXEC,
    BreakPointType_READ,
    BreakPointType_WRITE,
    BreakPointType_MAX,
};

struct Breakpoint
{
    bool enabled = false;
    uint16_t address = 0;
    LynxMemBank bank = LynxMemBank_RAM;
    BreakPointType type = BreakPointType_EXEC;
};

struct CallStackItem
{
    int src_address;
    int dst_address;
    bool is_breakpoint;
};

enum SessionStatus
{
    SessionStatus_None = 0,
    SessionStatus_Break,
    SessionStatus_Step,
    SessionStatus_Step_Out,
    SessionStatus_Step_Over,
    SessionStatus_Running,
    SessionStatus_Quit,
};

class Session : public std::enable_shared_from_this<Session>
{
    friend App;
    friend ControlsEditor;

  public:
    Session(std::filesystem::path file);
    ~Session();

    bool initialize(std::shared_ptr<VulkanRenderer> renderer);
    int id();
    std::string identifier();

    void register_main_screen();
    void unregister_main_screen();
    void render_screen(int screen_id, uint8_t *dispram, uint8_t *palette);
    ImTextureID get_main_screen_imgui_texture_id();
    std::shared_ptr<VulkanRenderer> &renderer();

    void set_status(SessionStatus status);
    SessionStatus status();
    std::shared_ptr<CSystem> system();
    uint8_t *palette();

    Symbols &symbols();

    std::vector<Breakpoint> &breakpoints();
    void add_breakpoint(uint16_t addr, LynxMemBank bank, BreakPointType type);
    void delete_breakpoint(uint16_t addr, LynxMemBank bank, BreakPointType type);
    void toggle_breakpoint(uint16_t addr, LynxMemBank bank, BreakPointType type);

    StatesManager &states_manager();

    ULONG execute();
    void destroy();

    std::filesystem::path cartridge_file();

    std::unordered_map<int, LynxButtons> &buttons_mapping();
    void set_button_status(int glfw_key, int mods, bool status);

    void register_app(std::shared_ptr<App> app);
    void enable_comlynx();
    void disable_comlynx();
    bool is_comlynx_enabled();

    void set_audio(bool enabled);
    bool is_audio_enabled();

    void set_active();
    bool is_active();

    std::vector<CallStackItem> &callstack();

  private:
    static int idinc;
    int _id;
    int _main_screen_id;
    uint8_t _palette[32];
    SessionStatus _status = SessionStatus_Break;
    Symbols _symbols{};
    StatesManager _states_manager{};
    std::filesystem::path _cartridge_file{};
    std::string _cartridge_file_name{};
    std::filesystem::path _rom_file = "./lynxboot.img";
    std::shared_ptr<App> _app;
    std::shared_ptr<CSystem> _lynx = nullptr;
    std::shared_ptr<VulkanRenderer> _renderer;
    std::vector<Breakpoint> _breakpoints{};
    std::vector<CallStackItem> _callstack{50};
    std::unordered_map<int, LynxButtons> _buttons_mapping = {{265, LynxButtons_Up},
                                                             {264, LynxButtons_Down},
                                                             {263, LynxButtons_Left},
                                                             {262, LynxButtons_Right},
                                                             {81, LynxButtons_Outside},
                                                             {87, LynxButtons_Inside},
                                                             {49, LynxButtons_Option1},
                                                             {50, LynxButtons_Option2},
                                                             {80, LynxButtons_Pause}};

    bool check_for_breakpoints();
    void memory_access_callback(LynxMemBank bank, uint16_t addr, bool write);
};

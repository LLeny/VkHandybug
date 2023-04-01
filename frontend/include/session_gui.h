#pragma once

#include "global.h"
#include "session.h"
#include "cpu_editor.h"
#include "palette_editor.h"
#include "watch_editor.h"
#include "memory_editor.h"
#include "symbols_editor.h"
#include "disasm_editor.h"
#include "cart_info_editor.h"
#include "breakpoints_editor.h"
#include "states_editor.h"
#include "controls_editor.h"
#include "session_control_editor.h"
#include "mikey_editor.h"

class Config;
class Menu;

struct SessionSingleEditor
{
    std::string icon;
    std::string label;
    IEditor *editor;
    bool *is_open;
    bool always_open = false;
};

class SessionGUI
{
    friend Config;
    friend Menu;

  public:
    SessionGUI();
    ~SessionGUI();
    std::string id();

    void initialize(std::shared_ptr<Session> session);
    void render();

    void add_memory_editor(int i);
    void add_disasm_editor(int i);

  private:
    std::shared_ptr<Session> _session;
    std::vector<SessionSingleEditor> _single_editors{};

    CPUEditor _cpu_editor{};
    PaletteEditor _palette_editor{};
    WatchEditor _watch_editor{};
    SymbolsEditor _symbols_editor{};
    CartInfoEditor _cart_info_editor{};
    BreakpointsEditor _breakpoints_editor{};
    StatesEditor _states_editor{};
    ControlsEditor _controls_editor{};
    SessionControlEditor _session_control_editor{};
    MikeyEditor _mikie_editor{};
    std::vector<std::shared_ptr<MemEditor>> _mem_editors{};
    std::vector<std::shared_ptr<DisasmEditor>> _disasm_editors{};

    bool _cart_info_open = false;
    bool _cpu_open = false;
    bool _palette_open = false;
    bool _watch_open = false;
    bool _symbols_open = false;
    bool _breakpoints_open = false;
    bool _states_manager_open = false;
    bool _controls_open = false;
    bool _mikie_open = false;

    void render_single_editors();
    void render_screen();
    void render_memory();
    void render_disassembly();
};
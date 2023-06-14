#include "session.h"
#include "log.h"
#include "symbols.h"
#include "config.h"
#include "vk_renderer.h"
#include "app.h"
#include <fmt/core.h>

int Session::idinc = 1;

Session::Session(std::filesystem::path file)
    : _cartridge_file{file}, _id{Session::idinc++}, _status{SessionStatus_Break}

{
    _cartridge_file_name = _cartridge_file.filename().generic_string();
}

Session::~Session()
{
}

bool Session::initialize(std::shared_ptr<VulkanRenderer> renderer)
{
    _renderer = renderer;
    _rom_file = Config::get_instance().store().lynx_rom_file;

    try
    {
        _lynx = std::make_shared<CSystem>(_cartridge_file.generic_string().c_str(), _rom_file.generic_string().c_str(), false);
    }
    catch (CLynxException &err)
    {
        LOG(LOGLEVEL_ERROR) << "Session - initialize() " << err.mMsg << ": " << err.mDesc;
        return false;
    }

    set_lynx_version(Config::get_instance().store().default_lynx_version);

    _status = SessionStatus_Break;

    _lynx->DisplaySetAttributes([](void *userdata, uint8_t *dispram, uint8_t *palette) {
        Session *session = (Session *)userdata;
        session->render_screen(session->_main_screen_id, dispram, palette);
        memcpy(session->_palette, palette, 32);
    },
                                this);

    auto symbol_file = _cartridge_file;
    symbol_file.replace_extension("lbl");

    if (std::filesystem::exists(symbol_file))
    {
        _symbols.load_symbols(symbol_file);
    }

    _states_manager.initialize(shared_from_this());
    _scripting.initialize(shared_from_this());

    _lynx->RegisterMemoryAccessCallback([&](LynxMemBank bank, uint16_t addr, bool write) { memory_access_callback(bank, addr, write); });

    return true;
}

void Session::memory_access_callback(LynxMemBank bank, uint16_t addr, bool write)
{
    BreakPointType type = write ? BreakPointType_WRITE : BreakPointType_READ;

    if (std::any_of(_breakpoints.begin(), _breakpoints.end(), [bank, addr, type](const Breakpoint &bp) { return bp.enabled && bp.address == addr && bp.bank == bank && bp.type == type; }))
    {
        set_status(SessionStatus_Break);
    }
}

void Session::register_app(std::shared_ptr<App> app)
{
    _app = app;
}

Symbols &Session::symbols()
{
    return _symbols;
}

int Session::id()
{
    return _id;
}

std::shared_ptr<CSystem> Session::system()
{
    return _lynx;
}

uint8_t *Session::palette()
{
    return _palette;
}

std::string Session::identifier()
{
    return fmt::format("{} {}", _cartridge_file_name, _id);
}
void Session::register_main_screen()
{
    _main_screen_id = _renderer->add_screen_view(0);
    _renderer->set_rotation(_main_screen_id, (int)_lynx->CartGetRotate());
}

void Session::unregister_main_screen()
{
    _renderer->delete_view(_main_screen_id);
}

ImTextureID Session::get_main_screen_imgui_texture_id()
{
    return _renderer->get_texture_ID(_main_screen_id);
}

void Session::render_screen(int screen_id, uint8_t *dispram, uint8_t *palette)
{
    _renderer->render_screen_view(screen_id, dispram, palette);
}

void Session::set_status(SessionStatus status)
{
    _status = status;
}

std::unordered_map<int, LynxButtons> &Session::buttons_mapping()
{
    return _buttons_mapping;
}

void Session::set_active()
{
    _app->set_active_session(identifier());
}

bool Session::is_active()
{
    return _app->is_active_session(identifier());
}

void Session::set_button_status(int glfw_key, int mods, bool status)
{
    if (mods || !_buttons_mapping.contains(glfw_key))
    {
        return;
    }

    auto buttons = _lynx->GetButtonData();
    auto rotation = _lynx->CartGetRotate();

    switch (_buttons_mapping[glfw_key])
    {
    case LynxButtons_Up:
        switch (rotation)
        {
        case CART_ROTATE_LEFT:
            buttons.sys.joystick.Bits.Left = status;
            break;
        case CART_ROTATE_RIGHT:
            buttons.sys.joystick.Bits.Right = status;
            break;
        default:
            buttons.sys.joystick.Bits.Up = status;
        }
        break;
    case LynxButtons_Down:
        switch (rotation)
        {
        case CART_ROTATE_LEFT:
            buttons.sys.joystick.Bits.Right = status;
            break;
        case CART_ROTATE_RIGHT:
            buttons.sys.joystick.Bits.Left = status;
            break;
        default:
            buttons.sys.joystick.Bits.Down = status;
        }
        break;
    case LynxButtons_Left:
        switch (rotation)
        {
        case CART_ROTATE_LEFT:
            buttons.sys.joystick.Bits.Down = status;
            break;
        case CART_ROTATE_RIGHT:
            buttons.sys.joystick.Bits.Up = status;
            break;
        default:
            buttons.sys.joystick.Bits.Left = status;
        }
        break;
    case LynxButtons_Right:
        switch (rotation)
        {
        case CART_ROTATE_LEFT:
            buttons.sys.joystick.Bits.Up = status;
            break;
        case CART_ROTATE_RIGHT:
            buttons.sys.joystick.Bits.Down = status;
            break;
        default:
            buttons.sys.joystick.Bits.Right = status;
        }
        break;
    case LynxButtons_Outside:
        buttons.sys.joystick.Bits.Outside = status;
        break;
    case LynxButtons_Inside:
        buttons.sys.joystick.Bits.Inside = status;
        break;
    case LynxButtons_Option1:
        buttons.sys.joystick.Bits.Option1 = status;
        break;
    case LynxButtons_Option2:
        buttons.sys.joystick.Bits.Option2 = status;
        break;
    case LynxButtons_Pause:
        buttons.sys.switches.Bits.Pause = status;
        break;
    }

    _lynx->SetButtonData(buttons);
}

SessionStatus Session::status()
{
    return _status;
}

std::vector<CallStackItem> &Session::callstack()
{
    return _callstack;
}

void Session::destroy()
{
    _status = SessionStatus_Quit;
    unregister_main_screen();
    disable_comlynx();
    _lynx = nullptr;
}

bool Session::check_for_breakpoints()
{
    C6502_REGS regs;
    _lynx->GetRegs(regs);

    auto found = std::find_if(_breakpoints.begin(), _breakpoints.end(), [regs](const Breakpoint &bp) { return bp.enabled && bp.type == BreakPointType_EXEC && (bp.address == 0 || bp.address == regs.PC); });

    while (found != _breakpoints.end())
    {
        if (found->script.empty())
        {
            LOG(LOGLEVEL_INFO) << "Session '" << identifier() << fmt::format("' breakpoint reached. PC = 0x{:04X}", regs.PC);
            set_status(SessionStatus_Break);
            return true;
        }
        else
        {
            auto id = found->identifier();
            if (_scripting.evaluate_breakpoint(id))
            {
                LOG(LOGLEVEL_INFO) << "Session '" << identifier() << "' breakpoint reached. Condition = " << found->script;
                set_status(SessionStatus_Break);
                return true;
            }
        }

        found = std::find_if(++found, _breakpoints.end(), [regs](const Breakpoint &bp) { return bp.enabled && bp.type == BreakPointType_EXEC && (bp.address == 0 || bp.address == regs.PC); });
    }

    return false;
}

ULONG Session::execute()
{
    ULONG cycles = 0;
    C6502_REGS regs;
    UBYTE opcode = 0;

    switch (_status)
    {
    case SessionStatus_Running:
    case SessionStatus_Step:
    case SessionStatus_Step_Out:
    case SessionStatus_Step_Over: {
        _lynx->GetRegs(regs);
        opcode = system()->Peek_CPU(regs.PC);
    }
    break;
    default:
        return cycles;
    }

    switch (opcode)
    {
    case JSR: {
        bool isbp = _status == SessionStatus_Step_Over && std::none_of(_callstack.begin(), _callstack.end(), [](const CallStackItem &i) { return i.is_breakpoint; });
        _callstack.push_back({regs.PC,
                              _lynx->PeekW_CPU(regs.PC + 1),
                              isbp});
    }
    break;
    case RTS: {
        if (_callstack.size() < 1)
        {
            break;
        }
        if (_callstack.back().is_breakpoint || _status == SessionStatus_Step_Out)
        {
            set_status(SessionStatus_Break);
        }
        _callstack.pop_back();
    }
    break;
    default:
        break;
    }

    try
    {
        cycles = _lynx->Update();
    }
    catch (CLynxException &ex)
    {
        if (ex.Error() == LynxErrors_Undocumented_Opcode && Config::get_instance().store().break_on_undocumented_opcode)
        {
            LOG(LOGLEVEL_INFO) << fmt::format("Session '{}' {}", identifier(), ex.Message().str());
            set_status(SessionStatus_Break);
        }
        else if (ex.Error() == LynxErrors_Illegal_Opcode)
        {
            LOG(LOGLEVEL_ERROR) << fmt::format("Session '{}' {}", identifier(), ex.Message().str());
            set_status(SessionStatus_Break);
        }
    }

    check_for_breakpoints();

    switch (_status)
    {
    case SessionStatus_Step:
        set_status(SessionStatus_Break);
        break;
    default:
        break;
    }

    return cycles;
}

std::vector<Breakpoint> &Session::breakpoints()
{
    return _breakpoints;
}

void Session::toggle_breakpoint(uint16_t addr, LynxMemBank bank, BreakPointType type, std::string &cond)
{
    if (std::any_of(_breakpoints.begin(), _breakpoints.end(), [addr, bank, type, cond](const Breakpoint &b) { return b.address == addr && b.bank == bank && b.type == type && b.script == cond; }))
    {
        delete_breakpoint(addr, bank, type, cond);
    }
    else
    {
        add_breakpoint(addr, bank, type, cond);
    }
}

void Session::add_breakpoint(uint16_t addr, LynxMemBank bank, BreakPointType type, std::string &cond)
{
    if (std::any_of(_breakpoints.begin(), _breakpoints.end(), [addr, bank, type, cond](const Breakpoint &b) { return b.address == addr && b.bank == bank && b.type == type && b.script == cond; }))
    {
        return;
    }

    Breakpoint bp(true, addr, bank, type, cond);
    _breakpoints.push_back(bp);
    set_breakpoint_script(bp.id, bp.script);
}

void Session::delete_breakpoint(uint16_t addr, LynxMemBank bank, BreakPointType type, std::string &cond)
{
    std::erase_if(_breakpoints, [addr, bank, type, cond](const Breakpoint &b) { return b.address == addr && b.bank == bank && b.type == type && b.script == cond; });
}

std::filesystem::path Session::cartridge_file()
{
    return _cartridge_file;
}

StatesManager &Session::states_manager()
{
    return _states_manager;
}

void Session::enable_comlynx()
{
    return _app->comlynx_hub().register_session(shared_from_this());
}

void Session::disable_comlynx()
{
    return _app->comlynx_hub().unregister_session(shared_from_this());
}

bool Session::is_comlynx_enabled()
{
    return _lynx->mMikie->ComLynxCable();
}

std::shared_ptr<VulkanRenderer> &Session::renderer()
{
    return _renderer;
}

void Session::set_audio(bool enabled)
{
    _lynx->mAudioEnabled = enabled;
    if (enabled)
    {
        sound_set_session(shared_from_this());
    }
}

bool Session::is_audio_enabled()
{
    return _lynx->mAudioEnabled;
}

void Session::set_breakpoint_script(std::string &id, std::string &script)
{
    _scripting.set_breakpoint_script(id, script);
}

bool Session::evaluate_breakpoint(std::string &scriptid)
{
    return _scripting.evaluate_breakpoint(scriptid);
}

void Session::set_lynx_version(LynxVersion_ ver)
{
    _lynx_version = ver;
    _lynx->SetLynxVersion(ver);
    LOG(LOGLEVEL_INFO) << fmt::format("Session '{}' version set to: {}", identifier(), (int)ver);
}

LynxVersion_ Session::get_lynx_version()
{
    return _lynx_version;
}
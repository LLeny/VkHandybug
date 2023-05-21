#include "scripting.h"
#include "log.h"
#include "session.h"

struct REGS_l
{
    REGS_l(std::function<C6502_REGS()> get, std::function<void(C6502_REGS)> set)
    {
        _get = get;
        _set = set;
    }

    sol::object get(sol::stack_object key, sol::this_state L)
    {
        auto regptr = key.as<sol::optional<std::string>>();

        if (!regptr.has_value())
        {
            return sol::object(L, sol::in_place, sol::lua_nil);
        }

        std::string r = *regptr;

        if (r == "A")
        {
            return sol::object(L, sol::in_place, _get().A);
        }
        if (r == "X")
        {
            return sol::object(L, sol::in_place, _get().X);
        }
        if (r == "Y")
        {
            return sol::object(L, sol::in_place, _get().Y);
        }
        if (r == "PC")
        {
            return sol::object(L, sol::in_place, _get().PC);
        }
        if (r == "PS")
        {
            return sol::object(L, sol::in_place, _get().PS);
        }
        if (r == "SP")
        {
            return sol::object(L, sol::in_place, _get().SP);
        }

        return sol::object(L, sol::in_place, sol::lua_nil);
    }

    void set(sol::stack_object key, sol::stack_object value, sol::this_state)
    {
        auto regptr = key.as<sol::optional<std::string>>();
        auto val = value.as<sol::optional<uint16_t>>();

        if (!regptr.has_value() || !val.has_value())
        {
            return;
        }

        std::string r = *regptr;

        auto reg = _get();

        if (r == "A")
        {
            reg.A = val.value();
        }
        else if (r == "X")
        {
            reg.X = val.value();
        }
        else if (r == "Y")
        {
            reg.Y = val.value();
        }
        else if (r == "PC")
        {
            reg.PC = val.value();
        }
        else if (r == "PS")
        {
            reg.PS = val.value();
        }
        else if (r == "SP")
        {
            reg.SP = val.value();
        }

        _set(reg);
    }

    std::function<C6502_REGS()> _get;
    std::function<void(C6502_REGS)> _set;
};

struct MEM_l
{
    MEM_l(std::function<uint8_t(uint16_t)> peek, std::function<void(uint16_t, uint8_t)> poke)
    {
        _peek = peek;
        _poke = poke;
    }

    sol::object get(sol::stack_object key, sol::this_state L)
    {
        auto addrptr = key.as<sol::optional<uint16_t>>();

        if (!addrptr.has_value())
        {
            return sol::object(L, sol::in_place, sol::lua_nil);
        }

        uint16_t addr = *addrptr;

        if (addr >= 0 && addr <= 0xFFFF)
        {
            return sol::object(L, sol::in_place, _peek(addr));
        }

        return sol::object(L, sol::in_place, sol::lua_nil);
    }

    void set(sol::stack_object key, sol::stack_object value, sol::this_state)
    {
        auto addrptr = key.as<sol::optional<uint16_t>>();
        auto val = value.as<sol::optional<uint8_t>>();

        if (!addrptr.has_value() || !val.has_value())
        {
            return;
        }

        uint16_t addr = *addrptr;

        if (addr >= 0 && addr <= 0xFFFF)
        {
            _poke(addr, val.value());
        }
    }

    std::function<uint8_t(uint16_t)> _peek;
    std::function<void(uint16_t, uint8_t)> _poke;
};

Scripting::Scripting()
{
}

Scripting::~Scripting()
{
}

bool Scripting::evaluate_breakpoint(std::string &scriptid)
{
    sol::function f = _lua_state[scriptid];

    if (!f.valid())
    {
        LOG(LOGLEVEL_ERROR) << "evaluate_breakpoint '" << scriptid << "' invalid.";
        return false;
    }

    try
    {
        return f();
    }
    catch (std::exception &e)
    {
        LOG(LOGLEVEL_ERROR) << "evaluate_breakpoint '" << scriptid << "' error: " << e.what();
        return false;
    }
}

void Scripting::set_breakpoint_script(std::string &id, std::string &script)
{
    auto scr = R"(
        function )" +
               id +
               R"(()
            return ()" +
               (script.empty() ? "true" : script) + R"()
        end
    )";

    try
    {
        _lua_state.safe_script(scr);
    }
    catch (const sol::error &e)
    {
        LOG(LOGLEVEL_ERROR) << "set_breakpoint_script error: " << e.what() << " '" << scr << "'";
    }
}

void Scripting::initialize(std::shared_ptr<Session> session)
{
    _session = session;

    _lua_state.set_exception_handler([](lua_State *L, sol::optional<const std::exception &> E, std::string_view S) {
        LOG(LOGLEVEL_ERROR) << "Lua script error: " << S;
        return sol::stack::push(L, S);
    });

    _lua_state.set_function("set_status", [&](SessionStatus s) { _session->set_status(s); });

    _lua_state.new_usertype<MEM_l>("MEM_l", sol::meta_function::index, &MEM_l::get, sol::meta_function::new_index, &MEM_l::set);
    _lua_state.new_usertype<REGS_l>("REGS_l", sol::meta_function::index, &REGS_l::get, sol::meta_function::new_index, &REGS_l::set);

    _lua_state["RAM"] = std::make_unique<MEM_l>([&](uint16_t addr) { return _session->system()->Peek_RAM(addr); }, [&](uint16_t addr, uint8_t v) { _session->system()->Poke_RAM(addr, v); });
    _lua_state["ROM"] = std::make_unique<MEM_l>([&](uint16_t addr) { return _session->system()->mRom->Peek(addr); }, [&](uint16_t addr, uint8_t v) { _session->system()->mRom->Poke(addr, v); });
    _lua_state["MIKEY"] = std::make_unique<MEM_l>([&](uint16_t addr) { return _session->system()->mMikie->Peek(addr); }, [&](uint16_t addr, uint8_t v) { _session->system()->mMikie->Poke(addr, v); });
    _lua_state["SUZY"] = std::make_unique<MEM_l>([&](uint16_t addr) { return _session->system()->mSusie->Peek(addr); }, [&](uint16_t addr, uint8_t v) { _session->system()->mSusie->Poke(addr, v); });
    _lua_state["CPU"] = std::make_unique<MEM_l>([&](uint16_t addr) { return _session->system()->Peek_CPU(addr); }, [&](uint16_t addr, uint8_t v) { _session->system()->Poke_CPU(addr, v); });
    _lua_state["CART"] = std::make_unique<MEM_l>([&](uint16_t addr) { return _session->system()->Peek_CART(addr); }, [&](uint16_t addr, uint8_t v) { _session->system()->Poke_CART(addr, v); });
    _lua_state["EEPROM"] = std::make_unique<MEM_l>([&](uint16_t addr) { return _session->system()->mEEPROM->Peek(addr); }, [&](uint16_t addr, uint8_t v) { _session->system()->mEEPROM->Poke(addr, v); });
    _lua_state["REGS"] = std::make_unique<REGS_l>([&]() { C6502_REGS regs; _session->system()->GetRegs(regs); return regs; }, [&](C6502_REGS regs) { _session->system()->SetRegs(regs); });
}
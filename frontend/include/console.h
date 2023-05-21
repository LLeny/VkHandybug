#pragma once

#include "global.h"
#include "imgui.h"
#include <sol/sol.hpp>

#define MAX_ITEMS 999

enum LOGLEVEL_
{
    LOGLEVEL_TRACE = 0,
    LOGLEVEL_DEBUG,
    LOGLEVEL_INFO,
    LOGLEVEL_WARN,
    LOGLEVEL_ERROR,
    LOGLEVEL_CMD,
    LOGLEVEL_ITEMCOUNT,
};

struct ConsoleItem
{
    LOGLEVEL_ level;
    std::string message;
};

class Console
{
  public:
    static Console &get_instance()
    {
        static Console instance;
        return instance;
    }
    Console(Console const &) = delete;
    void operator=(Console const &) = delete;

    bool render();

    void add_log(LOGLEVEL_ type, std::string msg);
    void exec_command(std::string cmd);
    int textedit_callback(ImGuiInputTextCallbackData *data);
    LOGLEVEL_ get_log_level()
    {
        return _log_level;
    }
    void set_log_level(LOGLEVEL_ lvl)
    {
        _log_level = lvl;
    }

  private:
    Console();

    void cmd_help();
    void cmd_history();
    void cmd_clear_log();

    char _inputBuf[256]{};
    std::deque<ConsoleItem> _items;
    std::vector<std::tuple<std::string, std::string>> _commands;
    std::vector<std::string> _history;
    int _historyPos = -1;
    ImGuiTextFilter _filter;
    bool _autoScroll = true;
    bool _scrollToBottom = true;
    sol::state _lua_state;
    std::mutex _mutex;
    LOGLEVEL_ _log_level = LOGLEVEL_WARN;

    const char* _level_prefixes[LOGLEVEL_ITEMCOUNT] = {"[TRC] ", "[DBG] ", "[INF] ", "[WRN] ", "[ERR] ", "[CMD] "};
    const char* _level_labels[LOGLEVEL_ITEMCOUNT] = {"Trace", "Debug", "Info", "Warning", "Error", "Command"};
};

class ConsoleLogger
{
  public:
    ConsoleLogger(LOGLEVEL_ lvl)
    {
        _log = Console::get_instance().get_log_level() <= lvl;
        _level = lvl;
    }

    ~ConsoleLogger()
    {
        if (!_log)
        {
            return;
        }
        Console::get_instance().add_log(_level, _buffer);
    }

    template <class T>
    ConsoleLogger &operator<<(const T &msg)
    {
        if (_log)
        {
            _buffer += msg;
        }
        return *this;
    }

  private:
    std::string _buffer{};
    LOGLEVEL_ _level;
    bool _log = true;
};

#pragma once

#include "global.h"
#include "imgui.h"

enum typelog
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CMD
};

struct ConsoleItem
{
    typelog level;
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

    void add_log(typelog type, std::string msg);
    void clear_log();
    void exec_command(std::string cmd);
    int textedit_callback(ImGuiInputTextCallbackData *data);
    std::string get_level_label(typelog level);

  private:
    Console();

    char _inputBuf[256]{};
    std::vector<ConsoleItem> _items;
    std::vector<std::tuple<std::string, std::string>> _commands;
    std::vector<std::string> _history;
    int _historyPos = -1;
    ImGuiTextFilter _filter;
    bool _autoScroll = true;
    bool _scrollToBottom = true;    
};

class ConsoleLogger
{
  public:
    ConsoleLogger(typelog lvl)
    {
        _level = lvl;
    }

    ~ConsoleLogger()
    {
        Console::get_instance().add_log(_level, _buffer);
    }

    template <class T>
    ConsoleLogger &operator<<(const T &msg)
    {
        _buffer += msg;
        return *this;
    }

  private:
    std::string _buffer{};
    typelog _level;
};

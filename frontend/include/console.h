#pragma once

#include "global.h"
#include "csys/system.h"
#include "imgui.h"
#include "imgui_console.h"

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

    csys::ItemLog &logger(csys::ItemType type);
    bool initialized();

    void render();

  private:
    Console(){};

    std::shared_ptr<ImGuiConsole> _console;
    csys::ItemLog _emptylogger;
};

class ConsoleLogger
{
  public:
    ConsoleLogger(csys::ItemType type)
        : _logger{Console::get_instance().logger(type)}
    {
        _initialized = Console::get_instance().initialized(); // imgui_console constructor requires an imgui context
    }

    ~ConsoleLogger()
    {
        if (_initialized)
        {
            _logger << csys::endl;
        }
    }

    template <class T>
    ConsoleLogger &operator<<(const T &msg)
    {
        if (_initialized)
        {
            _logger << msg;
        }
        return *this;
    }

  private:
    csys::ItemLog &_logger;
    bool _initialized = false;

    std::string get_label(csys::ItemType level)
    {
        switch (level)
        {
        case csys::ItemType::COMMAND:
            return "CMD";
        case csys::ItemType::CSYS_ERROR:
            return "ERR";
        case csys::ItemType::INFO:
            return "INF";
        case csys::ItemType::LOG:
            return "LOG";
        case csys::ItemType::NONE:
            return "   ";
        case csys::ItemType::WARNING:
            return "WRN";
        }
    }
};

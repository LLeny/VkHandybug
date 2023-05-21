#pragma once

#include "global.h"
#include "console.h"
#include <cstdlib>
#include <cxxabi.h>

class LOG
{
  public:
    LOG(LOGLEVEL_ type)
        : _console_logger(type)
    {
    }

    template <class T>
    LOG &operator<<(const T &msg)
    {
        _console_logger << msg;
        return *this;
    }

  private:
    ConsoleLogger _console_logger;
};
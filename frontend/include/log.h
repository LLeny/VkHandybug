#pragma once

#include "global.h"
#include "console.h"
#include <cstdlib>

class LOG
{
  public:
    LOG(typelog type)
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
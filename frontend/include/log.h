#pragma once

#include "global.h"
#include "console.h"
#include <cstdlib>
#include <cxxabi.h>

enum typelog
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

class LOG
{
  public:
    LOG(typelog type)
        : _console_logger(get_csys_level(type))
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

    csys::ItemType get_csys_level(typelog level)
    {
        switch (level)
        {
        case LOG_DEBUG:
            return csys::ItemType::LOG;
        case LOG_INFO:
            return csys::ItemType::INFO;
        case LOG_WARN:
            return csys::ItemType::WARNING;
        case LOG_ERROR:
            return csys::ItemType::CSYS_ERROR;
        }
    }
};
#pragma once

#include <iostream>

enum typelog
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

typedef struct structlog
{
    bool headers = false;
    typelog level = LOG_ERROR;
} structlog;

class LOG
{
  public:
    LOG()
    {
    }
    LOG(typelog type)
    {
        msglevel = type;
        operator<<("[" + getLabel(type) + "]");
    }
    ~LOG()
    {
        if (opened)
        {
            std::cout << std::endl;
        }
        opened = false;
    }
    template <class T>
    LOG &operator<<(const T &msg)
    {
        if (msglevel >= LOGCFG.level)
        {
            std::cout << msg;
            opened = true;
        }
        return *this;
    }

  private:
    structlog LOGCFG{};
    bool opened = false;
    typelog msglevel = LOG_ERROR;
    inline std::string getLabel(typelog type)
    {
        std::string label;
        switch (type)
        {
        case LOG_DEBUG:
            label = "DEBUG";
            break;
        case LOG_INFO:
            label = "INFO ";
            break;
        case LOG_WARN:
            label = "WARN ";
            break;
        case LOG_ERROR:
            label = "ERROR";
            break;
        }
        return label;
    }
};
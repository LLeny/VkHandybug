#pragma once

#include "global.h"
#include <sol/sol.hpp>

class Session;

class Scripting
{
  public:
    Scripting();
    ~Scripting();

    void initialize(std::shared_ptr<Session> session);
    void set_breakpoint_script(std::string &id, std::string &script);

    bool evaluate_breakpoint(std::string &scriptid);

  private:
    std::shared_ptr<Session> _session;
    sol::state _lua_state;
};

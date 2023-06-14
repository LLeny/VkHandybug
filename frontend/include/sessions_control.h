#pragma once

#include "global.h"
#include "imgui.h"

class App;

class SessionsControl
{
  public:
    SessionsControl();
    ~SessionsControl();

    void set_app(std::shared_ptr<App> app);

    bool render();

  private:
    std::shared_ptr<App> _app;
    bool create_button(std::string text, ImVec2 size, std::string tooltip);
};
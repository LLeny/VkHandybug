#pragma once

#include "global.h"

class App;

class Settings
{
  public:
    Settings();
    ~Settings();

    void initialize(std::shared_ptr<App> app);
    bool render();
    bool update_pending();
    void set_update_pending(bool update);

  private:
    std::shared_ptr<App> _app;

    bool _update_pending = false;

    void toggle_button(const char *str_id, bool *v);
};
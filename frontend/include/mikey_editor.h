#pragma once

#include "global.h"
#include "editor.h"

class MikeyEditor : public IEditor
{
  public:
    MikeyEditor();
    ~MikeyEditor() override;

    void render() override;

  private:
    void render_timers();
    void render_audio();
    void render_timer(uint8_t timerod);
};
#pragma once

#include "global.h"
#include "editor.h"

class SuzyEditor : public IEditor
{
  public:
    SuzyEditor();
    ~SuzyEditor() override;

    void render() override;

  private:
    void render_sprite();
    void render_math();
    void render_control();
};
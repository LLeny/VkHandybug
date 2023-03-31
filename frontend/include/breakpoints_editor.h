
#pragma once

#include "global.h"
#include "editor.h"

class Session;
class Config;
class Breakpoint;

class BreakpointsEditor : public IEditor
{
    friend Config;

  public:
    BreakpointsEditor();
    ~BreakpointsEditor() override;

    void render() override;

  private:
    void render_header();
    void render_table();
    void render_entry(Breakpoint &bp);

    char _address_buf[50]{};
};

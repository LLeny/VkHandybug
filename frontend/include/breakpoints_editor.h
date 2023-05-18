
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

    BreakPointType type_get_type(const char *type);
    const char *type_get_desc(BreakPointType type) const;

    char _address_buf[50]{};
    char _script_buf[501]{};
    LynxMemBank _mem_bank = LynxMemBank_RAM;
    BreakPointType _type = BreakPointType_EXEC;
};

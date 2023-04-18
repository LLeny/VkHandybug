
#pragma once

#include "global.h"
#include "editor.h"
#include "session.h"

class CallstackEditor : public IEditor
{
  public:
    CallstackEditor();
    ~CallstackEditor() override;

    void render() override;

  private:
    void draw_callstack_entry(CallStackItem &symbol);
};
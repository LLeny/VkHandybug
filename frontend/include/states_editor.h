#pragma once

#include "global.h"
#include "editor.h"
#include "states_manager.h"

class StatesEditor : public IEditor
{
  public:
    StatesEditor();
    ~StatesEditor() override;

    void render() override;

  private:
    void render_header();
    void render_state_item(State &item);
    void snap_state();
    void load_state(State &item);
    void delete_state(State &state);
};
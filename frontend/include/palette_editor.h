#pragma once

#include "global.h"
#include "session.h"
#include "system.h"
#include "editor.h"

class PaletteEditor : public IEditor
{
  public:
    PaletteEditor();
    ~PaletteEditor() override;

    void render() override;

  private:
    void draw_palette_entry(int id, uint16_t entry);
};
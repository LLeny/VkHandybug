
#pragma once

#include "global.h"
#include "editor.h"
#include "symbols.h"

class SymbolsEditor : public IEditor
{
  public:
    SymbolsEditor();
    ~SymbolsEditor() override;

    void render() override;

  private:
    void draw_symbol_entry(uint16_t addr, Symbol &symbol);
};
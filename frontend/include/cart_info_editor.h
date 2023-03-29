#pragma once

#include "global.h"
#include "editor.h"

class CartInfoEditor : public IEditor
{
  public:
    CartInfoEditor();
    ~CartInfoEditor() override;

    void render() override;
};
#pragma once

#include "global.h"
#include "session.h"
#include "system.h"
#include "editor.h"

class CPUEditor : public IEditor
{
  public:
    CPUEditor();
    ~CPUEditor() override;

    void render() override;

  private:
    bool _N = {false};
    bool _V = {false};
    bool _D = {false};
    bool _I = {false};
    bool _Z = {false};
    bool _C = {false};

    void draw_register(const char *label, uint8_t reg);
    void draw_PS(const char *label, uint16_t ps);
    void draw_flag(const char *label, bool enabled, bool *b);

    float LABEL_WIDTH = ImGui::GetFontSize() * 2;
    float ITEM_WIDTH = ImGui::GetFontSize() * 2.5;
};
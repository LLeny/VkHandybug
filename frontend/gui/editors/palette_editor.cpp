
#include "palette_editor.h"
#include "imgui.h"
#include <fmt/core.h>

PaletteEditor::PaletteEditor()
{
}

PaletteEditor::~PaletteEditor()
{
}

void PaletteEditor::render()
{
    if (!enabled())
    {
        return;
    }

    uint16_t *palette = (uint16_t *)_session->palette();

    if (ImGui::BeginTable("##paltable", 16, ImGuiTableFlags_SizingFixedSame))
    {
        for (int i = 0; i < 16; ++i)
        {
            ImGui::TableNextColumn();
            draw_palette_entry(i, *palette++);
        }
        ImGui::TableNextRow();
        for (int i = 0; i < 16; ++i)
        {
            ImGui::TableNextColumn();
            ImGui::Text("%02X", i);
        }
        ImGui::EndTable();
    }
}

void PaletteEditor::draw_palette_entry(int id, uint16_t entry)
{
    std::string label = fmt::format("##paletteentry{}", id);

    uint8_t g = entry >> 8;
    uint8_t br = entry & 0xFF;

    uint8_t gf = (g << 4) | (g & 0x0f);
    uint8_t rf = (br << 4) | (br & 0x0f);
    uint8_t bf = (br >> 4) | (br & 0xf0);

    float font_size = ImGui::GetFontSize();

    ImGui::ColorButton(label.c_str(), {(float)rf / 255.0f, (float)gf / 255.0f, (float)bf / 255.0f, 1.0f}, ImGuiColorEditFlags_None, {font_size, font_size});
}
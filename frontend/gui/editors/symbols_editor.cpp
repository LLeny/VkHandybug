
#include "symbols_editor.h"
#include "imgui.h"
#include "session.h"

SymbolsEditor::SymbolsEditor()
{
}

SymbolsEditor::~SymbolsEditor()
{
}

void SymbolsEditor::render()
{
    if (!enabled())
    {
        return;
    }

    if (ImGui::BeginTable("##symbolsitems", 2, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Address");
        ImGui::TableSetupColumn("Label");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (uint16_t i = 0; i < 0xffff; ++i)
        {
            draw_symbol_entry(i, _session->symbols().get_symbol(i));
        }

        ImGui::EndTable();
    }
}

void SymbolsEditor::draw_symbol_entry(uint16_t addr, Symbol &symbol)
{
    if (!symbol.override)
    {
        return;
    }

    ImGui::TableNextColumn();
    ImGui::Text("$%04X", addr);
    ImGui::TableNextColumn();
    ImGui::Text("%s", symbol.symbol.c_str());
}
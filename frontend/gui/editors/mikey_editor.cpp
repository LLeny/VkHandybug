#include "mikey_editor.h"
#include "fmt/core.h"

MikeyEditor::MikeyEditor()
{
}

MikeyEditor::~MikeyEditor()
{
}

void MikeyEditor::render()
{
    render_timers();
    render_audio();
}

void MikeyEditor::render_timers()
{
    ImGui::Text("Timers");
    if (ImGui::BeginTable("##mikeytimertable", 5, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
    {

        ImGui::TableSetupColumn("Timer");
        ImGui::TableSetupColumn("Backup");
        ImGui::TableSetupColumn("Static control");
        ImGui::TableSetupColumn("Current");
        ImGui::TableSetupColumn("Dynamic control");
        ImGui::TableHeadersRow();

        for (int8_t i = 0; i < 8; ++i)
        {
            render_timer(i);
        }

        ImGui::EndTable();
    }
}
void MikeyEditor::render_audio()
{
    ImGui::Text("Audio");
}
void MikeyEditor::render_timer(uint8_t timerid)
{
    auto mikie = _session->system()->mMikie;

    ImGui::TableNextColumn();
    ImGui::Text("%d", timerid);

    ImGui::TableNextColumn();
    imgui_char_hex("##TIMBKP", *mikie, timerid * 4 + 00, [&]() { return enabled(); });

    ImGui::TableNextColumn();
    imgui_char_bin("##TIMCTLA", *mikie, timerid * 4 + 01, [&]() { return enabled(); });

    ImGui::TableNextColumn();
    imgui_char_hex("##TIMCRT", *mikie, timerid * 4 + 02, [&]() { return enabled(); });

    ImGui::TableNextColumn();
    imgui_char_bin("##TIMCTLB", *mikie, timerid * 4 + 03, [&]() { return enabled(); });
}
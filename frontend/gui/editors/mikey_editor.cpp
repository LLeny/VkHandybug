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
    ImGui::Text("%02X", mikie->Peek(timerid * 4 + 00));

    ImGui::TableNextColumn();
    ImGui::Text("%s", fmt::format("{:08B}", mikie->Peek(timerid * 4 + 01)).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%02X", mikie->Peek(timerid * 4 + 02));

    ImGui::TableNextColumn();
    ImGui::Text("%s", fmt::format("{:08B}", mikie->Peek(timerid * 4 + 03)).c_str());
}
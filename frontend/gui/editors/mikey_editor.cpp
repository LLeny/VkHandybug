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
    ImGui::AlignTextToFramePadding();
    if (ImGui::CollapsingHeader("Timers"))
    {
        render_timers();
    }
    if (ImGui::CollapsingHeader("Audio"))
    {
        render_audio();
    }
    if (ImGui::CollapsingHeader("UART"))
    {
        render_comlynx();
    }
    if (ImGui::CollapsingHeader("Misc"))
    {
        render_misc();
    }
}

void MikeyEditor::render_comlynx()
{
    if (ImGui::BeginTable("##mikeycmtable", 6, ImGuiTableFlags_SizingFixedFit))
    {
        auto mikie = _session->system()->mMikie;

        ImGui::TableNextColumn();
        ImGui::Text("TXRDY");
        ImGui::TableNextColumn();
        bool txrdy = mikie->mUART_TX_COUNTDOWN & UART_TX_INACTIVE > 0;
        ImGui::Checkbox("##txrdy", &txrdy);

        ImGui::TableNextColumn();
        ImGui::Text("RXRDY");
        ImGui::TableNextColumn();
        bool rxrdy = mikie->mUART_RX_READY > 0;
        ImGui::Checkbox("##rxrdy", &rxrdy);

        ImGui::TableNextColumn();
        ImGui::Text("TXEMPTY");
        ImGui::TableNextColumn();
        bool txempty = mikie->mUART_TX_COUNTDOWN & UART_TX_INACTIVE > 0;
        ImGui::Checkbox("##txempty", &txempty);

        ImGui::TableNextColumn();
        ImGui::Text("PARERR");
        ImGui::TableNextColumn();
        bool parerr = false;
        ImGui::Checkbox("##parerr", &parerr);

        ImGui::TableNextColumn();
        ImGui::Text("OVERRUN");
        ImGui::TableNextColumn();
        bool ovrrun = mikie->mUART_Rx_overun_error > 0;
        ImGui::Checkbox("##ovrrun", &ovrrun);

        ImGui::TableNextColumn();
        ImGui::Text("FRAMERR");
        ImGui::TableNextColumn();
        bool framerr = mikie->mUART_Rx_framing_error > 0;
        ImGui::Checkbox("##framerr", &framerr);

        ImGui::TableNextColumn();
        ImGui::Text("RXBRK");
        ImGui::TableNextColumn();
        bool rxbrk = mikie->mUART_RX_DATA & UART_BREAK_CODE > 0;
        ImGui::Checkbox("##rxbrk", &rxbrk);

        ImGui::TableNextColumn();
        ImGui::Text("PARBIT");
        ImGui::TableNextColumn();
        bool par = mikie->mUART_RX_DATA & 0x0100 > 0;
        ImGui::Checkbox("##par", &par);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("SERDAT");
        ImGui::TableNextColumn();
        ImGui::Text("TX %2X / RX %2X", mikie->mUART_TX_DATA, mikie->mUART_RX_DATA);

        ImGui::EndTable();
    }
}

void MikeyEditor::render_timers()
{
    if (ImGui::BeginTable("##mikeytimertable", 5, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
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
    if (ImGui::BeginTable("##mikeyaudiotable", 9, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit))
    {
        ImGui::TableSetupColumn("Audio");
        ImGui::TableSetupColumn("Volume");
        ImGui::TableSetupColumn("Feedback");
        ImGui::TableSetupColumn("Output");
        ImGui::TableSetupColumn("Low shift");
        ImGui::TableSetupColumn("backup");
        ImGui::TableSetupColumn("Control");
        ImGui::TableSetupColumn("Counter");
        ImGui::TableSetupColumn("Others");
        ImGui::TableHeadersRow();

        for (int8_t i = 0; i < 4; ++i)
        {
            render_audio(i);
        }

        ImGui::EndTable();
    }
}

void MikeyEditor::render_misc()
{
    auto mikie = _session->system()->mMikie;

    imgui_char_bin("INTRST", *mikie, 0x80, [&]() { return is_read_only(); });
    ImGui::SameLine();
    imgui_char_bin("INTSET", *mikie, 0x81, [&]() { return is_read_only(); });
}

void MikeyEditor::render_timer(uint8_t timerid)
{
    auto mikie = _session->system()->mMikie;

    ImGui::TableNextColumn();
    ImGui::Text("%d", timerid);

    uint16_t base_addr = timerid * 4;

    ImGui::TableNextColumn();
    imgui_char_hex("##TIMBKP", *mikie, base_addr, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_bin("##TIMCTLA", *mikie, base_addr + 01, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_hex("##TIMCRT", *mikie, base_addr + 02, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_bin("##TIMCTLB", *mikie, base_addr + 03, [&]() { return is_read_only(); });
}

void MikeyEditor::render_audio(uint8_t audioid)
{
    auto mikie = _session->system()->mMikie;

    ImGui::TableNextColumn();
    ImGui::Text("%d", audioid);

    uint16_t base_addr = 0x20 + audioid * 8;

    ImGui::TableNextColumn();
    imgui_char_hex("##AUDVOL", *mikie, base_addr, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_bin("##AUDFEED", *mikie, base_addr + 01, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_hex("##AUDOUT", *mikie, base_addr + 02, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_bin("##AUDSHIFT", *mikie, base_addr + 03, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_hex("##AUDBKUP", *mikie, base_addr + 04, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_bin("##AUDCTLA", *mikie, base_addr + 05, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_hex("##AUDCNT", *mikie, base_addr + 06, [&]() { return is_read_only(); });

    ImGui::TableNextColumn();
    imgui_char_bin("##AUDCTLB", *mikie, base_addr + 07, [&]() { return is_read_only(); });
}
#include "suzy_editor.h"
#include "fmt/core.h"

SuzyEditor::SuzyEditor()
{
}

SuzyEditor::~SuzyEditor()
{
}

void SuzyEditor::render()
{
    ImGui::AlignTextToFramePadding();
    if (ImGui::CollapsingHeader("Sprite"))
    {
        render_sprite();
    }
    if (ImGui::CollapsingHeader("Math"))
    {
        render_math();
    }
    if (ImGui::CollapsingHeader("Control"))
    {
        render_control();
    }
}

void SuzyEditor::render_sprite()
{
    const std::string spr_str[] = {"", "TMPADR", "", "TILTACUM", "", "HOFF", "VOFF", "", "VIDBAS", "COLLBAS", "", "VIDADR", "COLLADR", "", "SCBNEXT", "", "SPRDLINE", "", "HPOSSTRT", "VPOSSTRT", "", "SPRHSIZ", "SPRVSIZ", "", "STRETCH", "TILT", "", "SPRDOFF", "SPRVPOS", "", "COLLOFF", "", "VSIZACUM", "", "HSIZOFF", "VSIZOFF", "", "SCBADR", "", "PROCADR"};
    const uint16_t spr_addr[] = {0, 0xFC00, 0, 0xFC02, 0, 0xFC04, 0xFC06, 0, 0xFC08, 0xFC0A, 0, 0xFC0C, 0xFC0E, 0, 0xFC10, 0, 0xFC12, 0, 0xFC14, 0xFC16, 0, 0xFC18, 0xFC1A, 0, 0xFC1C, 0xFC1E, 0, 0xFC20, 0xFC22, 0, 0xFC24, 0, 0xFC26, 0, 0xFC28, 0xFC2A, 0, 0xFC2C, 0, 0xFC2E};

    auto suzy = _session->system()->mSusie;

    if (ImGui::BeginTable("##sprs", 4, ImGuiTableFlags_SizingFixedSame))
    {
        for (int i = 0; i < sizeof(spr_str) / sizeof(spr_str[0]); ++i)
        {
            if (spr_str[i].empty())
            {
                ++i;
                ImGui::TableNextRow();
            }

            ImGui::TableNextColumn();
            imgui_char_hex(
                spr_str[i] + "L", *suzy, spr_addr[i], [&]() { return is_read_only(); }, 100);
            ImGui::TableNextColumn();
            imgui_char_hex(
                spr_str[i] + "H", *suzy, spr_addr[i] + 1, [&]() { return is_read_only(); }, 100);
        }

        ImGui::EndTable();
    }

    imgui_char_bin("SPRCTL0", *suzy, 0xFC80, [&]() { return is_read_only(); });
    ImGui::SameLine();
    imgui_char_bin("SPRCTL1", *suzy, 0xFC81, [&]() { return is_read_only(); });
    imgui_char_bin("SPRCOLL", *suzy, 0xFC82, [&]() { return is_read_only(); });
    imgui_char_bin("SPRINIT", *suzy, 0xFC83, [&]() { return is_read_only(); });
    imgui_char_bin("SPRSYS ", *suzy, 0xFC92, [&]() { return is_read_only(); });
}

void SuzyEditor::render_math()
{
    const std::string math_str[] = {"", "MATHD", "MATHC", "MATHB", "MATHA", "", "MATHP", "MATHN", "", "MATHH", "MATHG", "MATHF", "MATHE", "", "MATHM", "MATHL", "MATHK", "MATHJ"};
    const uint16_t math_addr[] = {0, 0xFC52, 0xFC53, 0xFC54, 0xFC55, 0, 0xFC56, 0xFC57, 0, 0xFC60, 0xFC61, 0xFC62, 0xFC63, 0, 0xFC6C, 0xFC6D, 0xFC6E, 0xFC6F};

    auto suzy = _session->system()->mSusie;

    for (int i = 0; i < sizeof(math_str) / sizeof(math_str[0]); ++i)
    {
        if (math_str[i].empty())
        {
            ++i;
        }
        else
        {
            ImGui::SameLine();
        }
        imgui_char_hex(math_str[i], *suzy, math_addr[i], [&]() { return is_read_only(); });
    }
}

void SuzyEditor::render_control()
{
    auto suzy = _session->system()->mSusie;

    imgui_char_bin("JOYSTICK", *suzy, 0xFCB0, [&]() { return is_read_only(); });
    ImGui::SameLine();
    imgui_char_bin("SWITCHES", *suzy, 0xFCB1, [&]() { return is_read_only(); });
}
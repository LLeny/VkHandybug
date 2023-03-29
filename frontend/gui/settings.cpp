#include "settings.h"
#include "config.h"
#include "imgui_internal.h"
#include "imgui.h"
#include <fmt/core.h>
#include "ImGuiFileDialog.h"

Settings::Settings()
{
}

Settings::~Settings()
{
}

bool Settings::render()
{
    if (ImGui::BeginTable("#settingstable", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableNextColumn();
        ImGui::Text("Dark theme");

        ImGui::TableNextColumn();
        bool darktheme = Config::getInstance().store().theme == "dark";
        toggle_button("##darkthemetoggle", &darktheme);
        if (darktheme)
        {
            Config::getInstance().store().theme = "dark";
        }
        else
        {
            Config::getInstance().store().theme = "light";
        }

        ImGui::TableNextColumn();
        ImGui::Text("Lynx ROM");

        ImGui::TableNextColumn();
        std::string buf(1024, '\0');
        strcpy(buf.data(), Config::getInstance().store().lynx_rom_file.data());
        if (ImGui::InputText("##lynxromfile", buf.data(), buf.length()))
        {
            Config::getInstance().store().lynx_rom_file = fmt::format("{}", buf);
        }
        ImGui::SameLine();
        if (ImGui::Button("..."))
        {
            ImGuiFileDialog::Instance()->OpenDialog("ChooseROM", "ROM", ".img,.rom,.bin", Config::getInstance().store().lynx_rom_file, 1, nullptr, ImGuiFileDialogFlags_Modal);
        }
        ImGui::EndTable();
    }

    if (ImGuiFileDialog::Instance()->Display("ChooseROM", ImGuiWindowFlags_None, {600, 300}))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            Config::getInstance().store().lynx_rom_file = ImGuiFileDialog::Instance()->GetFilePathName();
        }
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGui::Button("OK", {50, 25}))
    {
        return false;
    }

    return true;
}

bool Settings::update_pending()
{
    return _update_pending;
}

void Settings::set_update_pending(bool update)
{
    _update_pending = update;
}

void Settings::initialize(std::shared_ptr<App> app)
{
    _app = app;
}

void Settings::toggle_button(const char *str_id, bool *v)
{
    ImVec4 *colors = ImGui::GetStyle().Colors;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;
    ImGuiContext &gg = *GImGui;
    float ANIM_SPEED = 0.085f;
    if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id)) // && g.LastActiveIdTimer < ANIM_SPEED)
        float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
    if (ImGui::IsItemHovered())
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
    else
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}
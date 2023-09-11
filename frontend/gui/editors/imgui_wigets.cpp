#include "global.h"
#include "editor.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "fmt/core.h"

bool imgui_autocomplete_input(std::string label, char *buffer, size_t buffer_size, std::vector<std::string> &dictionary, ImGuiInputTextFlags flags)
{
    bool is_input_text_enter_pressed = ImGui::InputText(label.c_str(), buffer, buffer_size, ImGuiInputTextFlags_EnterReturnsTrue | flags);

    if (dictionary.size() <= 0)
    {
        return is_input_text_enter_pressed;
    }

    const bool is_input_text_active = ImGui::IsItemActive();
    const bool is_input_text_activated = ImGui::IsItemActivated();

    if (is_input_text_activated)
    {
        ImGui::OpenPopup("##popup");
    }

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
    ImGui::SetNextWindowSize({ImGui::GetItemRectSize().x * 1.5f, ImGui::GetTextLineHeight() * 8});
    if (ImGui::BeginPopup("##popup", ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_ChildWindow))
    {
        for (auto &word : dictionary)
        {
            if (strstr(word.c_str(), buffer) == NULL)
            {
                continue;
            }

            if (ImGui::Selectable(word.c_str()))
            {
                ImGui::ClearActiveID();
                strcpy(buffer, word.c_str());
            }
        }

        if (is_input_text_enter_pressed || (!is_input_text_active && !ImGui::IsWindowFocused()))
        {
            ImGui::CloseCurrentPopup();
            is_input_text_enter_pressed = true;
        }

        ImGui::EndPopup();
    }

    return is_input_text_enter_pressed;
}

bool imgui_char_hex(std::string label, IMemoryAccess &mem, uint16_t address, std::function<bool()> enabled, float label_width)
{
    auto buf = fmt::format("{:02X}", mem.Peek(address));

    if (!label.starts_with("##"))
    {
        ImGui::Text("%s", label.c_str());
        ImGui::SameLine(label_width);
    }

    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 2);
    if (!ImGui::InputText(("##charhex" + std::to_string(address)).c_str(), buf.data(), buf.length() + 1, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
    {
        return false;
    }

    uint8_t v;
    std::from_chars(buf.data(), buf.data() + buf.length(), v, 16);
    mem.Poke(address, v);
    return true;
}

int imgui_char_bin_edit_callback(ImGuiInputTextCallbackData *data)
{
    if (data->EventChar == '0' || data->EventChar == '1')
    {
        return 0;
    }
    return 1;
}

bool imgui_char_bin(std::string label, IMemoryAccess &mem, uint16_t address, std::function<bool()> enabled, float label_width)
{
    auto buf = fmt::format("{:08B}", mem.Peek(address));

    if (!label.starts_with("##"))
    {
        ImGui::Text("%s", label.c_str());
        ImGui::SameLine(label_width);
    }

    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 5);
    if (!ImGui::InputText(("##charbin" + std::to_string(address)).c_str(), buf.data(), buf.length() + 1, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CallbackCharFilter, imgui_char_bin_edit_callback))
    {
        return false;
    }

    uint8_t v;
    std::from_chars(buf.data(), buf.data() + buf.length(), v, 2);
    mem.Poke(address, v);
    return true;
}

LynxMemBank mem_bank_get_type(const char *data_type)
{
    for (int i = 0; i < LynxMemBank_MAX; ++i)
    {
        auto t = mem_bank_get_desc((LynxMemBank)i);

        while (*data_type or *t)
        {
            char c = std::toupper(*data_type++);
            char d = std::toupper(*t++);
            if (c != d)
                continue;
        }
        return (LynxMemBank)i;
    }
    return LynxMemBank_MIN;
}

const char *mem_bank_get_desc(LynxMemBank data_type)
{
    const char *descs[] = {"RAM", "ROM", "Suzy", "Mikey", "CPU", "CART", "EEPROM"};
    IM_ASSERT(data_type >= 0 && data_type < LynxMemBank_MAX);
    return descs[data_type];
}
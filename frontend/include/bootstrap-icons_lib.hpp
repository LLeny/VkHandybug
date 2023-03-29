#pragma once

#include <imgui.h>
#include "bootstrap-icons_data.hpp"
#include "bootstrap-icons.h"

namespace BootstrapIcons
{
namespace Font
{
static const ImWchar ranges[] = {BootstrapIcons::Font_StartCode, BootstrapIcons::Font_EndCode, 0};
inline ImFont *Load(ImGuiIO &io, const float size, ImFontConfig *config)
{
    void *data = const_cast<unsigned int *>(FONT_DATA);
    if (config)
    {
        config->FontDataOwnedByAtlas = false;
        return io.Fonts->AddFontFromMemoryTTF(data, FONT_DATA_SIZE, size, config, ranges);
    }
    else
    {
        ImFontConfig dconf;
        dconf.FontDataOwnedByAtlas = false;
        return io.Fonts->AddFontFromMemoryTTF(data, FONT_DATA_SIZE, size, &dconf, ranges);
    }
}
} // namespace Font
} // namespace BootstrapIcons

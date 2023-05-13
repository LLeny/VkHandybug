#include "console.h"

csys::ItemLog &Console::logger(csys::ItemType type)
{
    if (initialized())
    {
        return _console->System().Log(type);
    }
    return _emptylogger;
}

bool Console::initialized()
{
    return _console != nullptr;
}

void Console::render()
{
    if (_console == nullptr)
    {
        _console = std::make_shared<ImGuiConsole>("Console"); // Requires an imgui context...
    }

    _console->Draw();
}
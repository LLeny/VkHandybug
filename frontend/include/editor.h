#pragma once

#include "global.h"
#include "session.h"

class IEditor
{
  public:
    virtual ~IEditor() = default;

    virtual void render() = 0;
    virtual bool enabled()
    {
        return _session != nullptr;
    };
    virtual bool is_read_only()
    {
        if (!enabled())
        {
            return true;
        }

        return _session->status() != SessionStatus_Break;
    };
    virtual void set_session(std::shared_ptr<Session> session)
    {
        _session = session;
    };

  protected:
    std::shared_ptr<Session> _session;
};

bool imgui_autocomplete_input(std::string label, char *buffer, size_t buffer_size, std::vector<std::string> &dictionary, ImGuiInputTextFlags flags);
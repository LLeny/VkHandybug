
#pragma once

#include "global.h"
#include <imgui.h>
#include "imgui_memory_editor.h"

class Session;
class Config;

class MemEditor
{
    friend Config;

  public:
    MemEditor(int id);
    ~MemEditor();

    int id();
    std::string identifer();

    void set_session(std::shared_ptr<Session> session);
    void draw_contents();
    bool enabled();

    void write_changes(uint16_t offset, ImU8 data);
    ImU8 read_mem(uint16_t offset);

    void draw_bank_button(LynxMemBank access);
    const char *bank_label(LynxMemBank access);
    uint16_t bank_upper_bound(LynxMemBank access);

  private:
    std::string _identifier;
    int _id;
    std::shared_ptr<Session> _session;
    MemoryEditor _memoryEditor;
    uint32_t _bankUppberBound = 0xFFFF + 1;
    LynxMemBank _memBank = LynxMemBank_RAM;

    bool is_read_only();
};
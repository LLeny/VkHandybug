
#pragma once

#include "global.h"
#include <imgui.h>
#include "imgui_memory_editor.h"

class Session;
class Config;

enum MemEditorBank
{
    MemEditorBank_MIN = 0,
    MemEditorBank_RAM = MemEditorBank_MIN,
    MemEditorBank_ROM = 1,
    MemEditorBank_Suzy = 2,
    MemEditorBank_Mikey = 3,
    MemEditorBank_CPU = 4,
    MemEditorBank_MAX = 5
};

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

    void draw_bank_button(MemEditorBank access);
    const char *bank_label(MemEditorBank access);
    uint16_t bank_upper_bound(MemEditorBank access);

  private:
    std::string _identifier;
    int _id;
    std::shared_ptr<Session> _session;
    MemoryEditor _memoryEditor;
    uint32_t _bankUppberBound = 0xFFFF + 1;
    MemEditorBank _memBank = MemEditorBank_RAM;

    bool is_read_only();
};

#pragma once

#include "global.h"
#include "editor.h"

class Session;
class Config;

struct DisasmEntry
{
    uint16_t base_address = 0;
    std::string base_address_symbol{};
    uint8_t data_length = 0;
    std::string data{};
    std::string opcode{};
    std::string operands{};
    bool is_pc = false;
    bool has_breakpoint = false;
};

class DisasmEditor
{
    friend Config;

  public:
    DisasmEditor(int id);
    ~DisasmEditor();

    int id();
    std::string identifer();

    void set_session(std::shared_ptr<Session> session);
    void draw_contents();
    bool enabled();

  private:
    int _id;
    std::string _identifier;
    std::shared_ptr<Session> _session;
    uint16_t _local_pc{};
    bool _follow_pc{};
    bool _show_labels{};
    char _address_buf[50]{};

    bool is_read_only();
    void draw_disasm_table();
    int draw_disasm_entry(DisasmEntry &entry);
    void draw_options();
    void scroll_up();
    void scroll_down();
    DisasmEntry disassemble(uint16_t addr);
    int16_t next_address(int addr);
};

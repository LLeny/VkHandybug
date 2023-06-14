#pragma once

#include "global.h"

struct Symbol
{
    bool override = false;
    uint16_t address;
    std::string symbol;
};

class Symbols
{
  public:
    Symbols();
    ~Symbols();

    bool load_symbols(std::filesystem::path symbol_file);
    Symbol &get_symbol(uint16_t addr);
    int get_addr(std::string symbol);
    std::vector<std::string> &overrides();

  private:
    std::array<Symbol, 0x10000> _symbols{};
    std::vector<std::string> _override_symbols{};
    std::regex _line_regex{};

    void parse_line(std::string line);
    void reset();
};
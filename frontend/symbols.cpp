#include "symbols.h"
#include "log.h"
#include <fmt/core.h>

Symbols::Symbols()
{
    _line_regex.assign(("([0-9a-fA-F]+)\\s+(.+)"));

    for (int i = 0; i < 0x100; ++i)
    {
        _symbols[i] = {false, (uint16_t)i, fmt::format("${:02X}", i)};
    }
    for (int i = 0x100; i <= 0xffff; ++i)
    {
        _symbols[i] = {false, (uint16_t)i, fmt::format("${:04X}", i)};
    }
}

Symbols::~Symbols()
{
}

bool Symbols::load_symbols(std::filesystem::path symbol_file)
{
    if (!std::filesystem::exists(symbol_file))
    {
        LOG(LOGLEVEL_ERROR) << "Symbols - could not find '" << symbol_file.generic_string() << "'";
        return false;
    }

    std::ifstream infile(symbol_file);

    std::string line;

    while (std::getline(infile, line))
    {
        parse_line(line);
    }

    std::sort(_override_symbols.begin(), _override_symbols.end());

    return true;
}

void Symbols::parse_line(std::string line)
{
    std::smatch match;
    std::regex_search(line, match, _line_regex);

    if (match.size() != 3)
    {
        return;
    }

    auto addr_match = match[1].str();
    auto label_match = match[2].str();

    int addr;
    std::from_chars(addr_match.data(), addr_match.data() + addr_match.size(), addr, 16);

    if (addr < 0 || addr > 0xffff)
    {
        return;
    }

    _symbols[addr] = {true, (uint16_t)addr, label_match};
    _override_symbols.push_back(label_match);
}

Symbol &Symbols::get_symbol(uint16_t addr)
{
    return _symbols[addr];
}

std::vector<std::string> &Symbols::overrides()
{
    return _override_symbols;
}

int Symbols::get_addr(std::string symbol)
{
    auto found = std::find_if(_symbols.begin(), _symbols.end(), [symbol](const Symbol &s) { return symbol == s.symbol; });

    if (found == _symbols.end())
    {
        return -1;
    }

    return found->address;
}
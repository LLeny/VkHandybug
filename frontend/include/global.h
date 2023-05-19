#pragma once

#include "../configure.h"

#include <array>
#include <algorithm>
#include <memory>
#include <vector>
#include <stack>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <exception>
#include <charconv>
#include <span>
#include <regex>
#include <iterator>
#include <future>

enum LynxButtons
{
    LynxButtons_Up = 0,
    LynxButtons_Down,
    LynxButtons_Left,
    LynxButtons_Right,
    LynxButtons_Outside,
    LynxButtons_Inside,
    LynxButtons_Option1,
    LynxButtons_Option2,
    LynxButtons_Pause,
    LynxButtons_Max
};

enum LynxMemBank
{
    LynxMemBank_MIN = 0,
    LynxMemBank_RAM = LynxMemBank_MIN,
    LynxMemBank_ROM = 1,
    LynxMemBank_Suzy = 2,
    LynxMemBank_Mikey = 3,
    LynxMemBank_CPU = 4,
    LynxMemBank_CART = 5,
    LynxMemBank_EEPROM = 6,
    LynxMemBank_MAX = 7
};

bool iequals(const std::string& a, const std::string& b);
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);
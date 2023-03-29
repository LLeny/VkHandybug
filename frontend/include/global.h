#pragma once

#include "../configure.h"

#include <array>
#include <algorithm>
#include <memory>
#include <vector>
#include <filesystem>
#include <fstream>
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
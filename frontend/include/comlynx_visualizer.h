#pragma once

#include "global.h"
#include "imgui.h"

#define MAX_FRAMES (300)

struct ComLynxFrame
{
    std::string source_identifier{};
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    uint8_t data{};
};

class ComLynxVisualizer
{
  public:
    ComLynxVisualizer();
    ~ComLynxVisualizer();

    bool render();
    void add_frame(std::string source_identifier, std::chrono::system_clock::time_point timestamp, uint8_t data);

  private:
    std::vector<ComLynxFrame> _frames{};
    std::unordered_map<std::string, bool> _known_sessions{};
    std::mutex _frame_lock{};
};
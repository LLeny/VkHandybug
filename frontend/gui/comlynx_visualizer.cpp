#include "comlynx_visualizer.h"
#include "fmt/core.h"
#include "fmt/chrono.h"

ComLynxVisualizer::ComLynxVisualizer()
{
}

ComLynxVisualizer::~ComLynxVisualizer()
{
}

bool ComLynxVisualizer::render()
{
    bool open = true;
    if (ImGui::Begin("ComLynx", &open))
    {
        if (ImGui::BeginTable("##comlynxtable", _known_sessions.size() + 1, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Timestamp");

            for (auto &session : _known_sessions)
            {
                ImGui::TableSetupColumn(session.first.c_str());
            }

            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            std::chrono::time_point<std::chrono::system_clock> prev_timestamp{};

            {
                std::scoped_lock<std::mutex> lock(_frame_lock);

                for (unsigned i = _frames.size(); i-- > 0;)
                {
                    auto frame = _frames[i];

                    ImGui::TableNextColumn();
                    if (prev_timestamp == frame.timestamp)
                    {
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(255, 0, 0, 255)));
                    }
                    ImGui::Text("%s", fmt::format("{:%T}", frame.timestamp).c_str());

                    for (auto &session : _known_sessions)
                    {
                        ImGui::TableNextColumn();
                        if (session.first == frame.source_identifier)
                        {
                            ImGui::Text("%s", fmt::format("{:2X}", frame.data).c_str());
                        }
                    }

                    prev_timestamp = frame.timestamp;
                }
            }

            ImGui::EndTable();
        }
    }

    ImGui::End();

    return open;
}

void ComLynxVisualizer::add_frame(std::string source_identifier, std::chrono::system_clock::time_point timestamp, uint8_t data)
{
    {
        std::scoped_lock<std::mutex> lock(_frame_lock);
        while (_frames.size() > MAX_FRAMES)
        {
            _frames.erase(_frames.begin());
        }
    }

    if (!_known_sessions.contains(source_identifier))
    {
        _known_sessions[source_identifier] = true;
    }

    _frames.push_back({source_identifier, timestamp, data});
}
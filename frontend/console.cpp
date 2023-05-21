#include "console.h"
#include "fmt/core.h"

Console::Console()
{
    _lua_state.set_exception_handler([](lua_State *L, sol::optional<const std::exception &> E, std::string_view S) {
        Console::get_instance().add_log(LOGLEVEL_ERROR, fmt::format("Lua: {}", S));
        return sol::stack::push(L, S);
    });

    _commands.push_back({"clear", "Clear the logs."});
    _commands.push_back({"history", "Diplay the command history."});
    _commands.push_back({"help", "Display the help."});
    _commands.push_back({"log 'some text'", "Print to the console."});

    _lua_state.set_function("log", [&](std::string msg) { add_log(LOGLEVEL_INFO, msg); });
    _lua_state.set_function("clear", [&]() { cmd_clear_log(); });
    _lua_state.set_function("help", [&]() { cmd_help(); });
    _lua_state.set_function("history", [&]() { cmd_history(); });
}

bool Console::render()
{
    bool open;

    ImGui::SetNextWindowSize(ImVec2(520, 300), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Console", &open))
    {
        ImGui::End();
        return open;
    }

    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Close Console"))
        {
            open = false;
        }
        ImGui::EndPopup();
    }

    if (ImGui::SmallButton("Clear"))
    {
        cmd_clear_log();
    }
    ImGui::SameLine();
    ImGui::TextWrapped("Enter 'help' for help.");

    ImGui::Separator();

    if (ImGui::BeginPopup("Options"))
    {
        ImGui::Checkbox("Auto-scroll", &_autoScroll);
        ImGui::EndPopup();
    }

    if (ImGui::Button("Options"))
    {
        ImGui::OpenPopup("Options");
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::BeginCombo("##consolelevel", _level_labels[_log_level], ImGuiComboFlags_HeightLargest))
    {
        for (int n = 0; n < LOGLEVEL_ITEMCOUNT; n++)
        {
            if (ImGui::Selectable(_level_labels[n], _log_level == n))
            {
                set_log_level((LOGLEVEL_)n);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    _filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear"))
            {
                cmd_clear_log();
            }
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

        {
            std::unique_lock<std::mutex> lock{_mutex};
            for (auto &item : _items)
            {
                auto msg = _level_prefixes[item.level] + item.message;

                if (!_filter.PassFilter(msg.c_str()))
                {
                    continue;
                }

                ImVec4 color;
                bool has_color = false;
                if (item.level == LOGLEVEL_ERROR)
                {
                    color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                    has_color = true;
                }
                else if (item.level == LOGLEVEL_CMD)
                {
                    color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
                    has_color = true;
                }
                if (has_color)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                }
                ImGui::TextUnformatted(msg.c_str());
                if (has_color)
                {
                    ImGui::PopStyleColor();
                }
            }
        }

        if (_scrollToBottom || (_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
        {
            ImGui::SetScrollHereY(1.0f);
        }
        _scrollToBottom = false;

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
    ImGui::Separator();

    bool reclaim_focus = false;
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
    if (ImGui::InputText(
            "##consoleinput", _inputBuf, sizeof(_inputBuf), input_text_flags, [](ImGuiInputTextCallbackData *data) { return Console::get_instance().textedit_callback(data); }, (void *)this))
    {
        std::string s = _inputBuf;
        if (!s.empty())
        {
            exec_command(s);
            strcpy(_inputBuf, "");
        }
        reclaim_focus = true;
    }

    ImGui::SetItemDefaultFocus();
    if (reclaim_focus)
    {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();

    return open;
}

void Console::add_log(LOGLEVEL_ type, std::string msg)
{
    std::unique_lock<std::mutex> lock{_mutex};
    if (_items.size() >= MAX_ITEMS)
    {
        _items.pop_front();
    }
    _items.push_back(ConsoleItem{type, msg});
}

void Console::cmd_clear_log()
{
    std::unique_lock<std::mutex> lock{_mutex};
    _items.clear();
}

void Console::cmd_help()
{
    add_log(LOGLEVEL_INFO, "Commands:");
    for (auto &hlpcmd : _commands)
    {
        const auto [c, h] = hlpcmd;
        add_log(LOGLEVEL_INFO, fmt::format("  {}: {}", c, h));
    }
};

void Console::cmd_history()
{
    int first = _history.size() - 10;
    for (int i = first > 0 ? first : 0; i < _history.size(); i++)
    {
        add_log(LOGLEVEL_INFO, fmt::format("  {}: {}", i, _history[i]));
    }
};

void Console::exec_command(std::string command)
{
    auto cmd = command;
    trim(cmd);

    add_log(LOGLEVEL_CMD, cmd);

    _historyPos = -1;
    for (int i = _history.size() - 1; i >= 0; i--)
    {
        if (iequals(_history[i], cmd))
        {
            _history.erase(_history.begin() + i);
            break;
        }
    }
    _history.push_back(cmd);

    if (std::any_of(_commands.begin(), _commands.end(), [&](std::tuple<std::string, std::string> &icmd) { const auto [c, h] = icmd;return iequals(cmd, c); }))
    {
        cmd += "()";
    }

    try
    {
        _lua_state.safe_script(cmd);
    }
    catch (const sol::error &e)
    {
        add_log(LOGLEVEL_ERROR, e.what());
        return;
    }

    _scrollToBottom = true;
}

int Console::textedit_callback(ImGuiInputTextCallbackData *data)
{
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion: {
        const char *word_end = data->Buf + data->CursorPos;
        const char *word_start = word_end;
        while (word_start > data->Buf)
        {
            const char c = word_start[-1];
            if (c == ' ' || c == '\t' || c == ',' || c == ';')
            {
                break;
            }
            word_start--;
        }

        std::vector<std::string> candidates;
        for (auto &hlpcmd : _commands)
        {
            const auto [c, h] = hlpcmd;
            auto len = (int)(word_end - word_start);
            auto inp = std::string(word_start, len);
            if (iequals(c.substr(0, len), inp))
            {
                candidates.push_back(c);
            }
        }

        if (candidates.empty())
        {
            auto inp = std::string(word_start, (int)(word_end - word_start));
            add_log(LOGLEVEL_WARN, fmt::format("No match for '{}'!", inp));
        }
        else if (candidates.size() == 1)
        {
            data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
            data->InsertChars(data->CursorPos, candidates[0].c_str());
            data->InsertChars(data->CursorPos, " ");
        }
        else
        {
            int match_len = (int)(word_end - word_start);
            for (;;)
            {
                int c = 0;
                bool all_candidates_matches = true;
                for (int i = 0; i < candidates.size() && all_candidates_matches; i++)
                    if (i == 0)
                    {
                        c = toupper(candidates[i][match_len]);
                    }
                    else if (c == 0 || c != toupper(candidates[i][match_len]))
                    {
                        all_candidates_matches = false;
                    }
                if (!all_candidates_matches)
                {
                    break;
                }
                match_len++;
            }

            if (match_len > 0)
            {
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
            }

            add_log(LOGLEVEL_INFO, "Possible matches:\n");
            for (auto &c : candidates)
            {
                add_log(LOGLEVEL_INFO, fmt::format("  {}", c));
            }
        }

        break;
    }
    case ImGuiInputTextFlags_CallbackHistory: {
        const int prev_history_pos = _historyPos;
        if (data->EventKey == ImGuiKey_UpArrow)
        {
            if (_historyPos == -1)
            {
                _historyPos = _history.size() - 1;
            }
            else if (_historyPos > 0)
            {
                _historyPos--;
            }
        }
        else if (data->EventKey == ImGuiKey_DownArrow)
        {
            if (_historyPos != -1)
            {
                if (++_historyPos >= _history.size())
                {
                    _historyPos = -1;
                }
            }
        }

        if (prev_history_pos != _historyPos)
        {
            const char *history_str = (_historyPos >= 0) ? _history[_historyPos].c_str() : "";
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, history_str);
        }
    }
    }
    return 0;
}

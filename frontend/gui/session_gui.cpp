#include "session_gui.h"
#include "imgui_internal.h"
#include "imgui.h"
#include "log.h"
#include <fmt/core.h>

SessionGUI::SessionGUI()
{
}

SessionGUI::~SessionGUI()
{
}

void SessionGUI::initialize(std::shared_ptr<Session> session)
{
    _session = session;

    auto sid = id();
    _display_window_id = fmt::format("{}##{}{}", "Display", "Display", sid);
    _cpu_window_id = fmt::format("{}##{}{}", "CPU", "CPU", sid);
    _watch_window_id = fmt::format("{}##{}{}", "Watches", "Watches", sid);
    _palette_window_id = fmt::format("{}##{}{}", "Palette", "Palette", sid);
    _symbols_window_id = fmt::format("{}##{}{}", "Symbols", "Symbols", sid);
    _cart_info_window_id = fmt::format("{}##{}{}", "Cart info", "Cart info", sid);
    _breakpoints_window_id = fmt::format("{}##{}{}", "Breakpoints", "Breakpoints", sid);
    _states_window_id = fmt::format("{}##{}{}", "States", "States", sid);
    _controls_window_id = fmt::format("{}##{}{}", "Joystick", "Joystick", sid);
    _session_control_window_id = fmt::format("{}##{}{}", "Control", "Control", sid);
    _mikey_window_id = fmt::format("{}##{}{}", "Mikey", "Mikey", sid);
    _suzy_window_id = fmt::format("{}##{}{}", "Suzy", "Suzy", sid);
    _callstack_window_id = fmt::format("{}##{}{}", "Callstack", "Callstack", sid);
    _memory_window_id = fmt::format("{}##{}{}", "Memory", "Memory", sid);
    _disasm_window_id = fmt::format("{}##{}{}", "Disassembly", "Disassembly", sid);

    _cpu_editor.set_session(session);
    _palette_editor.set_session(session);
    _watch_editor.set_session(session);
    _symbols_editor.set_session(session);
    _cart_info_editor.set_session(session);
    _breakpoints_editor.set_session(session);
    _states_editor.set_session(session);
    _controls_editor.set_session(session);
    _session_control_editor.set_session(session);
    _mikie_editor.set_session(session);
    _suzy_editor.set_session(session);
    _callstack_editor.set_session(session);

    _single_editors.push_back({_cpu_window_id, &_cpu_editor, &_cpu_open});
    _single_editors.push_back({_watch_window_id, &_watch_editor, &_watch_open});
    _single_editors.push_back({_palette_window_id, &_palette_editor, &_palette_open});
    _single_editors.push_back({_symbols_window_id, &_symbols_editor, &_symbols_open});
    _single_editors.push_back({_cart_info_window_id, &_cart_info_editor, &_cart_info_open});
    _single_editors.push_back({_breakpoints_window_id, &_breakpoints_editor, &_breakpoints_open});
    _single_editors.push_back({_states_window_id, &_states_editor, &_states_manager_open});
    _single_editors.push_back({_controls_window_id, &_controls_editor, &_controls_open});
    _single_editors.push_back({_session_control_window_id, &_session_control_editor, NULL, true});
    _single_editors.push_back({_mikey_window_id, &_mikie_editor, &_mikie_open});
    _single_editors.push_back({_suzy_window_id, &_suzy_editor, &_suzy_open});
    _single_editors.push_back({_callstack_window_id, &_callstack_editor, &_callstack_open});
}

std::string SessionGUI::id()
{
    return _session->identifier();
}

void SessionGUI::init_default_session(ImGuiID dockid)
{
    ImGui::DockBuilderRemoveNode(dockid);
    ImGui::DockBuilderAddNode(dockid);

    ImGuiID A, B, C, D, E, F, G, H, I;

    ImGuiID c2;
    ImGuiID c1 = ImGui::DockBuilderSplitNode(dockid, ImGuiDir_Left, 0.70f, NULL, &c2);
    ImGuiID c1r1;
    ImGuiID c1r2 = ImGui::DockBuilderSplitNode(c1, ImGuiDir_Down, 0.60f, NULL, &c1r1);
    ImGuiID c1r1c2;
    ImGuiID c1r1c1 = ImGui::DockBuilderSplitNode(c1r1, ImGuiDir_Left, 0.30f, NULL, &c1r1c2);
    ImGuiID c1r1c2r1;
    E = ImGui::DockBuilderSplitNode(c1r1c2, ImGuiDir_Down, 0.50f, NULL, &c1r1c2r1);
    B = ImGui::DockBuilderSplitNode(c1r1c1, ImGuiDir_Down, 0.30f, NULL, &A);
    C = ImGui::DockBuilderSplitNode(c1r1c2r1, ImGuiDir_Left, 0.50f, NULL, &D);
    F = ImGui::DockBuilderSplitNode(c1r2, ImGuiDir_Left, 0.60f, NULL, &G);
    I = ImGui::DockBuilderSplitNode(c2, ImGuiDir_Down, 0.15f, NULL, &H);

    ImGui::DockBuilderDockWindow(_palette_window_id.c_str(), A);
    ImGui::DockBuilderDockWindow(_display_window_id.c_str(), A);
    ImGui::DockBuilderDockWindow(_cart_info_window_id.c_str(), A);
    ImGui::DockBuilderDockWindow(_controls_window_id.c_str(), B);
    ImGui::DockBuilderDockWindow(_states_window_id.c_str(), B);
    ImGui::DockBuilderDockWindow(_session_control_window_id.c_str(), B);
    ImGui::DockBuilderDockWindow(_cpu_window_id.c_str(), C);
    ImGui::DockBuilderDockWindow(_callstack_window_id.c_str(), D);
    ImGui::DockBuilderDockWindow(_watch_window_id.c_str(), E);
    ImGui::DockBuilderDockWindow(_suzy_window_id.c_str(), F);
    ImGui::DockBuilderDockWindow(_mikey_window_id.c_str(), F);
    ImGui::DockBuilderDockWindow(_breakpoints_window_id.c_str(), I);
    ImGui::DockBuilderDockWindow(_symbols_window_id.c_str(), H);
    ImGui::DockBuilderDockWindow((_disasm_window_id + "Disassembly 1").c_str(), H);
    ImGui::DockBuilderDockWindow((_memory_window_id + "Memory 1").c_str(), G);

    ImGui::DockBuilderFinish(dockid);

    ImGui::SetWindowSize(ImVec2(300, 300));

    add_memory_editor(-1);
    add_disasm_editor(-1);
}

void SessionGUI::build_dock()
{
    auto sessiondockid = ImGui::GetID(id().c_str());

    if (ImGui::DockBuilderGetNode(sessiondockid) == NULL)
    {
        init_default_session(sessiondockid);
    }

    ImGui::DockSpace(sessiondockid);
}

void SessionGUI::load_symbols(std::string symbol_file)
{
    _session->load_symbols(symbol_file);
}

void SessionGUI::render()
{
    render_screen();
    render_memory();
    render_disassembly();
    render_single_editors();
}

void SessionGUI::add_memory_editor(int i)
{
    if (i < 0)
    {
        if (_mem_editors.size() <= 0)
        {
            i = 1;
        }
        else
        {
            auto max = std::max_element(_mem_editors.begin(), _mem_editors.end(), [](std::shared_ptr<MemEditor> a, std::shared_ptr<MemEditor> b) { return a->id() < b->id(); });
            i = (*max)->id() + 1;
        }
    }

    auto editor = std::make_shared<MemEditor>(i);

    editor->set_session(_session);

    _mem_editors.push_back(std::move(editor));
}

void SessionGUI::add_disasm_editor(int i)
{
    if (i < 0)
    {
        if (_disasm_editors.size() <= 0)
        {
            i = 1;
        }
        else
        {
            auto max = std::max_element(_disasm_editors.begin(), _disasm_editors.end(), [](std::shared_ptr<DisasmEditor> a, std::shared_ptr<DisasmEditor> b) { return a->id() < b->id(); });
            i = (*max)->id() + 1;
        }
    }

    auto editor = std::make_shared<DisasmEditor>(i);

    editor->set_session(_session);

    _disasm_editors.push_back(std::move(editor));
}

void SessionGUI::render_screen()
{
    ImVec2 contentSize;
    float ratio;

    if ((int)_session->system()->CartGetRotate())
    {
        contentSize = ImVec2{SCREEN_HEIGHT, SCREEN_WIDTH};
        ratio = (float)SCREEN_HEIGHT / SCREEN_WIDTH;
    }
    else
    {
        contentSize = ImVec2{SCREEN_WIDTH, SCREEN_HEIGHT};
        ratio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

    if (ImGui::Begin(_display_window_id.c_str(), NULL, flags))
    {
        auto size = ImGui::GetWindowSize();
        float headerHeight = ImGui::GetFrameHeight();
        ImVec2 frameSize = {size.x, size.y - headerHeight};

        if (auto tex = _session->get_main_screen_imgui_texture_id())
        {
            ImVec2 tgtSize = {frameSize};

            if (tgtSize.x / ratio > tgtSize.y)
            {
                tgtSize.x = (float)(int)(tgtSize.y * ratio);
            }
            else
            {
                tgtSize.y = (float)(int)(tgtSize.x / ratio);
            }

            ImVec2 pos = {(frameSize.x - tgtSize.x) / 2.0f, (frameSize.y - tgtSize.y) / 2.0f + headerHeight};
            ImGui::SetCursorPos(pos);
            ImGui::Image(tex, {tgtSize.x, tgtSize.y});
        }
    }

    ImGui::End();

    if (ImGui::IsWindowFocused())
    {
        _session->set_active();
    }
}

void SessionGUI::render_memory()
{
    for (auto memedit : _mem_editors)
    {
        bool open = true;

        auto memidentifier = memedit->identifer();

        std::string i = fmt::format("{}{}", _memory_window_id, memidentifier);

        if (ImGui::Begin(i.c_str(), &open, ImGuiWindowFlags_None))
        {
            memedit->draw_contents();

            if (ImGui::IsWindowFocused())
            {
                _session->set_active();
            }
        }
        ImGui::End();

        if (!open)
        {
            std::erase_if(_mem_editors, [memidentifier](std::shared_ptr<MemEditor> e) { return e->identifer() == memidentifier; });
        }
    }
}

void SessionGUI::render_disassembly()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5, 0});

    for (auto disasmedit : _disasm_editors)
    {
        bool open = true;

        auto identifier = disasmedit->identifer();

        std::string i = fmt::format("{}{}", _disasm_window_id, identifier);

        if (ImGui::Begin(i.c_str(), &open, ImGuiWindowFlags_None))
        {
            disasmedit->draw_contents();

            if (ImGui::IsWindowFocused())
            {
                _session->set_active();
            }
        }

        ImGui::End();

        if (!open)
        {
            std::erase_if(_disasm_editors, [identifier](std::shared_ptr<DisasmEditor> e) { return e->identifer() == identifier; });
        }
    }

    ImGui::PopStyleVar();
}

void SessionGUI::render_single_editors()
{
    for (auto &e : _single_editors)
    {
        if (!e.always_open && !*e.is_open)
        {
            continue;
        }

        if (ImGui::Begin(e.label.c_str(), e.is_open, ImGuiWindowFlags_None))
        {
            (*e.editor).render();

            if (ImGui::IsWindowFocused())
            {
                _session->set_active();
            }
        }
        ImGui::End();
    }
}

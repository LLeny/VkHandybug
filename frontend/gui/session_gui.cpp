#include "session_gui.h"
#include "imgui.h"
#include "log.h"
#include "bootstrap-icons.h"
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
    _cpu_editor.set_session(session);
    _palette_editor.set_session(session);
    _watch_editor.set_session(session);
    _symbols_editor.set_session(session);
    _cart_info_editor.set_session(session);
    _breakpoints_editor.set_session(session);
    _states_editor.set_session(session);
    _controls_editor.set_session(session);
    _session_control_editor.set_session(session);

    bool always_open = true;

    _single_editors.push_back({std::string(BootstrapIcons_cpu), id(), &_cpu_editor, &_cpu_open});
    _single_editors.push_back({std::string(BootstrapIcons_binoculars), id(), &_watch_editor, &_watch_open});
    _single_editors.push_back({std::string(BootstrapIcons_droplet), id(), &_palette_editor, &_palette_open});
    _single_editors.push_back({std::string(BootstrapIcons_globe), id(), &_symbols_editor, &_symbols_open});
    _single_editors.push_back({std::string(BootstrapIcons_info_circle), id(), &_cart_info_editor, &_cart_info_open});
    _single_editors.push_back({std::string(BootstrapIcons_bookmarks), id(), &_breakpoints_editor, &_breakpoints_open});
    _single_editors.push_back({std::string(BootstrapIcons_folder2_open), id(), &_states_editor, &_states_manager_open});
    _single_editors.push_back({std::string(BootstrapIcons_controller), id(), &_controls_editor, &_controls_open});
    _single_editors.push_back({std::string(BootstrapIcons_play), id(), &_session_control_editor, &always_open, true});
}

std::string SessionGUI::id()
{
    return _session->identifier();
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
    std::string i = fmt::format("{} {}", BootstrapIcons_display, id());

    ImVec2 contentSize;
    float ratio;
    bool open = true;

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

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {contentSize.x, contentSize.y});

    ImGui::Begin(i.c_str(), &open, flags);

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

    ImGui::PopStyleVar(3);

    if (ImGui::IsWindowFocused())
    {
        _session->set_active();
    }

    ImGui::End();
}

void SessionGUI::render_memory()
{
    for (auto memedit : _mem_editors)
    {
        bool open = true;

        auto memidentifier = memedit->identifer();

        std::string i = fmt::format("{} {} {}", BootstrapIcons_box_seam, id(), memidentifier);

        ImGui::Begin(i.c_str(), &open, ImGuiWindowFlags_None);

        memedit->draw_contents();

        if (ImGui::IsWindowFocused())
        {
            _session->set_active();
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

        std::string i = fmt::format("{} {} {}", BootstrapIcons_code, id(), identifier);

        ImGui::Begin(i.c_str(), &open, ImGuiWindowFlags_None);

        disasmedit->draw_contents();

        if (ImGui::IsWindowFocused())
        {
            _session->set_active();
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

        std::string i = fmt::format("{} {}", e.icon, e.label);

        ImGui::Begin(i.c_str(), e.is_open, ImGuiWindowFlags_None);

        (*e.editor).render();

        if (ImGui::IsWindowFocused())
        {
            _session->set_active();
        }
        ImGui::End();
    }
}

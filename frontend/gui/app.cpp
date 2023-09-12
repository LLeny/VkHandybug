#include "app.h"
#include "global.h"
#include "config.h"

App::App()
{
}

App::~App()
{
}

void App::main_loop()
{
    using frames = std::chrono::duration<int64_t, std::ratio<1, 75>>; // limit to 75Hz
    auto nextFrame = std::chrono::system_clock::now() + frames{0};
    auto lastFrame = nextFrame - frames{1};

    while (!_renderer->should_close() && !_closing)
    {
        render();

        std::this_thread::sleep_until(nextFrame);

        lastFrame = nextFrame;
        nextFrame += frames{1};
    }
}

std::shared_ptr<GUI> App::gui()
{
    return _gui;
}

void App::ask_to_close()
{
    _closing = true;
}

void App::close()
{
    _closing = true;
    _execute_thread.join();

    sound_stop();

    Config::get_instance().save_sessions(_gui->sessions());
    Config::get_instance().save(this);

    for (auto session : _sessions)
    {
        session->destroy();
    }
}

void App::close_session(std::string session_identifier)
{
    LOG(LOGLEVEL_DEBUG) << "App - close_session " << session_identifier;

    auto found = std::find_if(_sessions.begin(), _sessions.end(), [session_identifier](const std::shared_ptr<Session> &s) { return s->identifier() == session_identifier; });

    if (found == _sessions.end())
    {
        return;
    }

    auto session = *found;

    sound_clear_session(session_identifier);
    _gui->unregister_session(session_identifier);

    std::erase_if(_sessions, [session_identifier](const std::shared_ptr<Session> &s) { return s->identifier() == session_identifier; });

    session->destroy();
}

void App::initialize()
{
    Config::get_instance().load(this);

    _gui = std::make_shared<GUI>();
    _gui->initialize();
    _gui->sessions_control().set_app(shared_from_this());
    _gui->menu().set_app(shared_from_this());
    _gui->menu().register_file_open_callback([&](std::string file) { open_file(file); });

    _comlynx_hub.register_comlynx_visualizer(_gui->comlynx_visualizer());

    _renderer = std::make_shared<VulkanRenderer>();
    _renderer->register_file_open_callback([&](std::string file) { open_file(file); });
    _renderer->initialize();

    _renderer->setTitle(APP_NAME " " APP_VERSION);

    Config::get_instance().initialize(this);

    if (!sound_initialize())
    {
        LOG(LOGLEVEL_ERROR) << "Could not initialize sound.";
    }

    _renderer->register_key_event_callback([&](int glfw_key, int mods, bool glfw_action) {
        process_key_event(glfw_key, mods, glfw_action);
    });

    _execute_thread = std::thread([&]() { execute(); });
}

void App::render()
{
    _renderer->render(_gui);
}

int App::get_pressed_key()
{
    for (int i = 0; i < _key_statuses.size(); ++i)
    {
        if (_key_statuses[i])
        {
            return i;
        }
    }
    return -1;
}

void App::process_key_event(int glfw_key, int mods, bool glfw_action)
{
    if (!glfw_action)
    {
        _key_statuses[glfw_key] = 0;
    }
    else
    {
        _key_statuses[glfw_key] = mods | glfw_action;
    }

    for (auto &session : _sessions)
    {
        session->set_button_status(glfw_key, mods, glfw_action);
    }
}

std::vector<std::shared_ptr<Session>> App::sessions()
{
    return _sessions;
}

void App::set_active_session(std::string session_identifier)
{
    _active_session_identifier = session_identifier;
}

bool App::is_active_session(std::string session_identifier)
{
    return _active_session_identifier == session_identifier;
}

void App::open_file(std::string file)
{
    std::filesystem::path path(file);

    LOG(LOGLEVEL_INFO) << "App - opening '" << path.generic_string() << "'";

    if (!std::filesystem::exists(path))
    {
        LOG(LOGLEVEL_INFO) << "App - '" << path.generic_string() << "' doesn't exist...";
        return;
    }

    std::erase_if(_recent_sessions, [file](std::string r) { return r == file.c_str(); });

    while (_recent_sessions.size() > 5)
    {
        _recent_sessions.erase(_recent_sessions.begin());
    }

    _recent_sessions.push_back(file.c_str());

    new_session(path);
}

void App::execute()
{
    using frames = std::chrono::duration<int64_t, std::ratio<1, 18000000>>;
    auto sleepFrame = std::chrono::system_clock::now() + frames{0};
    auto targetFrame = sleepFrame + frames{0};
    int64_t count = 0;
    int max_cycles;
    int cycles[10]{};
    int cycle;

    while (!_closing)
    {
        sleepFrame = std::chrono::system_clock::now();
        max_cycles = 0;

        for (int i = 0; i < _sessions.size(); ++i)
        {
            auto &session = _sessions[i];

            if (!session || !session->_lynx)
            {
                continue;
            }

            cycles[i] = session->execute();

            max_cycles = std::max(max_cycles, cycles[i]);
        }

        if (!max_cycles)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else
        {
            for (int i = 0; i < _sessions.size(); ++i)
            {
                while (cycles[i] < max_cycles)
                {
                    cycle = _sessions[i]->execute();
                    if (!cycle)
                    {
                        cycles[i] = max_cycles;
                        continue;
                    }
                    cycles[i] += cycle;
                }
            }

            _comlynx_hub.process_queue();
            targetFrame = sleepFrame + frames{max_cycles};
            do
            {
                count++;
            } while (std::chrono::system_clock::now() < targetFrame);
        }
    }
}

void App::new_session(std::filesystem::path file)
{
    auto session = std::make_shared<Session>(file);

    session->register_app(shared_from_this());

    if (!session->initialize(_renderer))
    {
        LOG(LOGLEVEL_ERROR) << "App - couldn't initialize session '" << file.generic_string() << "'";
        return;
    }

    LOG(LOGLEVEL_INFO) << "App - session initialized '" << file.generic_string() << "'";

    session->register_main_screen();

    auto sessiongui = _gui->register_session(session);

    Config::get_instance().load_session(sessiongui);

    _sessions.push_back(std::move(session));
}

std::vector<std::string> &App::recent_sessions()
{
    return _recent_sessions;
}

ComLynxHub &App::comlynx_hub()
{
    return _comlynx_hub;
}

ImVec2 App::get_dimensions()
{
    return _renderer->get_dimensions();
}

ImVec2 App::get_position()
{
    return _renderer->get_position();
}

void App::set_dimensions(ImVec2 dim)
{
    _renderer->set_dimensions(dim);
}

void App::set_position(ImVec2 pos)
{
    _renderer->set_position(pos);
}
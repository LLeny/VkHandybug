#include "states_manager.h"
#include "session.h"
#include "md5.h"
#include "log.h"
#include <fmt/core.h>
#include <fmt/chrono.h>

StatesManager::StatesManager()
{
}

StatesManager::~StatesManager()
{
}

void StatesManager::initialize(std::shared_ptr<Session> session)
{
    _session = session;
    calc_cart_hash();
    update_states_list();
}

std::vector<State> &StatesManager::states()
{
    return _states;
}

void StatesManager::snap_state()
{
    auto dir = states_directory(true);

    if (dir.empty())
    {
        LOG(LOGLEVEL_ERROR) << "StatesManager - snap_state no state directory";
        return;
    }

    auto statefile = new_context_filename();

    if (statefile.empty())
    {
        LOG(LOGLEVEL_ERROR) << "StatesManager - snap_state no state file name";
        return;
    }

    if (!_session->system()->ContextSave(statefile.c_str()))
    {
        LOG(LOGLEVEL_ERROR) << "StatesManager - snap_state couldn't save state";
        return;
    }

    update_states_list();
}

std::string StatesManager::new_context_filename()
{
    auto dir = states_directory(false);

    if (dir.empty())
    {
        LOG(LOGLEVEL_INFO) << "StatesManager - new_context_filename no state directory";
        return {};
    }

    auto now = std::chrono::system_clock::now();

    dir /= fmt::format("{:%Y%m%d_%H%M%S}.sth", floor<std::chrono::seconds>(now));

    return dir.generic_string();
}

void StatesManager::load_state(State &state)
{
    if (!_session->system()->ContextLoad(state.file.generic_string().c_str()))
    {
        LOG(LOGLEVEL_ERROR) << "StatesManager - load_state couldn't load state '" << state.file.generic_string() << "'";
    }
}

void StatesManager::delete_state(State &state)
{
    if (!std::filesystem::remove(state.file))
    {
        LOG(LOGLEVEL_ERROR) << "StatesManager - delete_state couldn't delete state '" << state.file.generic_string() << "'";
    }
    update_states_list();
}

void StatesManager::calc_cart_hash()
{
    try
    {
        MD5 md5;
        std::ifstream file;
        std::istream *input = NULL;

        file.open(_session->cartridge_file().generic_string().c_str(), std::ios::in | std::ios::binary);
        if (!file)
        {
            LOG(LOGLEVEL_ERROR) << "StatesManager - session_directory_string can't open '" << _session->cartridge_file().generic_string() << "'";
            return;
        }
        input = &file;

        const size_t BufferSize = 1024;
        char buffer[BufferSize];

        while (*input)
        {
            input->read(buffer, BufferSize);
            std::size_t numBytesRead = size_t(input->gcount());
            md5.add(buffer, numBytesRead);
        }

        file.close();

        _cart_hash = md5.getHash();
    }
    catch (std::exception &e)
    {
        LOG(LOGLEVEL_ERROR) << "StatesManager - calc_cart_hash " << e.what();
        return;
    }
}

std::string StatesManager::states_directory_string()
{
    if (_cart_hash.empty())
    {
        LOG(LOGLEVEL_ERROR) << "StatesManager - states_directory_string, no cart hash";
        return {};
    }

    auto path = std::filesystem::path(".") / "states" / _cart_hash;
    return path.generic_string();
}

std::filesystem::path StatesManager::states_directory(bool forcecreate)
{
    std::string dirname = states_directory_string();

    if (dirname.empty())
    {
        LOG(LOGLEVEL_ERROR) << "StatesManager - session_directory, no dirname";
        return {};
    };

    if (!std::filesystem::exists(dirname))
    {
        if (forcecreate)
        {
            if (!std::filesystem::create_directories(dirname))
            {
                LOG(LOGLEVEL_ERROR) << "StatesManager - session_directory, couldn't create '" << dirname << "'";
                return {};
            }
        }
        else
        {
            return {};
        }
    }

    return std::filesystem::path(dirname);
}

void StatesManager::update_states_list()
{
    auto dir = states_directory(false);

    if (dir.empty())
    {
        LOG(LOGLEVEL_INFO) << "StatesManager - initialize_states no state directory";
        return;
    }

    _states.clear();

    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
        if (entry.is_directory() || entry.path().extension() != ".sth")
        {
            continue;
        }

        _states.push_back({entry.path()});
    }
}
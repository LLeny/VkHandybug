#pragma once

#include "global.h"

struct State
{
    std::filesystem::path file;
};

class Session;

class StatesManager
{
  public:
    StatesManager();
    ~StatesManager();

    void initialize(std::shared_ptr<Session> session);

    std::vector<State> &states();
    void snap_state();
    void load_state(State &state);
    void delete_state(State &state);

  private:
    std::shared_ptr<Session> _session;
    std::vector<State> _states{};
    std::string _cart_hash{};

    std::string states_directory_string();
    std::filesystem::path states_directory(bool forcecreate = false);
    std::string new_context_filename();

    void calc_cart_hash();
    void update_states_list();
};
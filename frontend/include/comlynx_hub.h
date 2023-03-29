#pragma once

#include "global.h"

class Session;
class ComLynxHub;
class ComLynxVisualizer;

struct ComLynxCallBackArg
{
    ComLynxHub *hub;
    std::string session_identifier;
};

struct ComLynxQueueItem
{
    std::string sender_id;
    int data;
};

class ComLynxHub
{
  public:
    ComLynxHub();
    ~ComLynxHub();

    void register_session(std::shared_ptr<Session> session);
    void unregister_session(std::shared_ptr<Session> session);
    void process_queue();
    void register_comlynx_visualizer(std::shared_ptr<ComLynxVisualizer> visualizer);

  private:
    std::shared_ptr<ComLynxVisualizer> _comlynx_visualizer;
    std::vector<std::shared_ptr<Session>> _sessions{};
    std::vector<std::shared_ptr<ComLynxCallBackArg>> _callback_objrefs{};
    std::vector<ComLynxQueueItem> _queue{};

    void enqueue(int data, std::string sender_id);
};

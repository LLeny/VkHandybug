#include "comlynx_hub.h"
#include "session.h"
#include "system.h"
#include "comlynx_visualizer.h"

ComLynxHub::ComLynxHub()
{
}

ComLynxHub::~ComLynxHub()
{
}

void ComLynxHub::register_session(std::shared_ptr<Session> session)
{
    auto objref = std::make_shared<ComLynxCallBackArg>();
    objref->hub = this;
    objref->session_identifier = session->identifier();
    _callback_objrefs.push_back(objref);

    session->system()->ComLynxTxCallback([](int data, void *objref) {
        ComLynxCallBackArg *arg = (ComLynxCallBackArg *)objref;
        arg->hub->enqueue(data, arg->session_identifier);
    },
                                         objref.get());

    session->system()->ComLynxCable(1);

    _sessions.push_back(session);
}

void ComLynxHub::register_comlynx_visualizer(std::shared_ptr<ComLynxVisualizer> visualizer)
{
    _comlynx_visualizer = visualizer;
}

void ComLynxHub::unregister_session(std::shared_ptr<Session> session)
{
    auto found = std::find_if(_sessions.begin(), _sessions.end(), [&](const std::shared_ptr<Session> s) { return s->identifier() == session->identifier(); });

    if (found == _sessions.end())
    {
        return;
    }

    (*found)->system()->ComLynxCable(0);
    (*found)->system()->ComLynxTxCallback(nullptr, nullptr);

    std::erase_if(_sessions, [&](const std::shared_ptr<Session> s) { return s->identifier() == session->identifier(); });
    std::erase_if(_callback_objrefs, [&](const std::shared_ptr<ComLynxCallBackArg> s) { return s->session_identifier == session->identifier(); });
}

void ComLynxHub::process_queue()
{
    if (_queue.empty())
    {
        return;
    }

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

    for (auto &f : _queue)
    {
        _comlynx_visualizer->add_frame(f.sender_id, now, (uint8_t)f.data);
    }

    if (_queue.size() == 1)
    {
        auto &item = _queue.back();

        for (auto session : _sessions)
        {
            if (session->identifier() == item.sender_id)
            {
                continue;
            }
            session->system()->ComLynxRxData(item.data);
        }
    }

    _queue.clear();
}

void ComLynxHub::enqueue(int data, std::string sender_id)
{
    _queue.push_back({sender_id, data});
}

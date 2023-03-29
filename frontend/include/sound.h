#pragma once

#include "global.h"

class Session;

bool sound_initialize();
void sound_stop();
void sound_set_session(std::shared_ptr<Session> session);
void sound_clear_session(std::string session_identifier);

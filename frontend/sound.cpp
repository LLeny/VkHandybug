#include "global.h"
#include "sound.h"
#include "session.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

ma_device_config _sound_config;
ma_device _sound_device;

void sound_clear_session(std::string session_identifier)
{
    if (_sound_config.pUserData && ((Session *)_sound_config.pUserData)->identifier() == session_identifier)
    {
        _sound_config.pUserData = nullptr;
    }
}

void sound_set_session(std::shared_ptr<Session> session)
{
    if (_sound_config.pUserData)
    {
        ((Session *)_sound_config.pUserData)->system()->mAudioEnabled = false;
    }

    _sound_config.pUserData = session.get();
    session->system()->mAudioEnabled = true;
}

void sound_data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
    if (!_sound_config.pUserData)
    {
        return;
    }
    Session *session = (Session *)_sound_config.pUserData;

    auto system = session->system();

    if (!system)
    {
        return;
    }

    int len = frameCount * 4;

    if (system->mSystemHalt || !system->mAudioBufferPointer || system->mAudioBufferPointer < len)
    {
        return;
    }

    memcpy(pOutput, system->mAudioBuffer, len);
    memmove(system->mAudioBuffer, system->mAudioBuffer + len, system->mAudioBufferPointer - len);
    system->mAudioBufferPointer = system->mAudioBufferPointer - len;
}

bool sound_initialize()
{
    _sound_config = ma_device_config_init(ma_device_type_playback);
    _sound_config.playback.format = ma_format_s16;
    _sound_config.playback.channels = 2;
    _sound_config.sampleRate = HANDY_AUDIO_SAMPLE_FREQ;
    _sound_config.dataCallback = sound_data_callback;
    _sound_config.pUserData = nullptr;

    if (ma_device_init(NULL, &_sound_config, &_sound_device) != MA_SUCCESS)
    {
        return false;
    }

    ma_device_start(&_sound_device);

    return true;
}

void sound_stop()
{
    ma_device_uninit(&_sound_device);
}

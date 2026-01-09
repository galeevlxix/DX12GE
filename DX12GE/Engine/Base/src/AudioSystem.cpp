#include "../AudioSystem.h"

AudioSystem::AudioSystem()
{
    ThrowIfFailed(
        XAudio2Create(xaudio.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR));

    ThrowIfFailed(
        xaudio->CreateMasteringVoice(&master));

    XAUDIO2_VOICE_DETAILS details{};
    master->GetVoiceDetails(&details);
    dstChannels = details.InputChannels;

    master->GetChannelMask(&channelMask);

    X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3d);
}

void AudioSystem::Shutdown()
{
    if (master) 
    { 
        master->DestroyVoice(); 
        master = nullptr; 
    }
    xaudio.Reset();
}

void AudioSystem::SetVolume(float vol)
{
    if (master && vol >= 0.0f && vol <= XAUDIO2_MAX_VOLUME_LEVEL)
    {
        master->SetVolume(vol);
    }
}

float AudioSystem::GetVolume()
{
    float volume = 0.0f;
    if (master)
    {
        master->GetVolume(&volume);
    }
    return volume;
}

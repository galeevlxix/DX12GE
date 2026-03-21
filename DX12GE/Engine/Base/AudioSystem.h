#pragma once

#include "DX12LibPCH.h"

struct AudioSystem
{
    ComPtr<IXAudio2> xaudio;
    IXAudio2MasteringVoice* master = nullptr;

    X3DAUDIO_HANDLE x3d = {};
    DWORD channelMask = 0;
    int dstChannels = 0;

    AudioSystem();

    void Shutdown();

    void SetVolume(float vol);

    float GetVolume();
};
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

#include <xaudio2.h>
#include <x3daudio.h>
#include <wrl/client.h>

#include "DX12LibPCH.h"

namespace bian_audio
{
    static constexpr float SPEED_OF_SOUND = X3DAUDIO_SPEED_OF_SOUND;

    struct AudioSystem
    {
        ComPtr<IXAudio2> xaudio;
        IXAudio2MasteringVoice* master = nullptr;

        X3DAUDIO_HANDLE x3d = {};
        DWORD channelMask = 0;
        int dstChannels = 0;

        bool Init()
        {
            ThrowIfFailed(
                XAudio2Create(xaudio.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR));

            ThrowIfFailed(
                xaudio->CreateMasteringVoice(&master));

            XAUDIO2_VOICE_DETAILS details{};
            master->GetVoiceDetails(&details);
            dstChannels = details.InputChannels;

            master->GetChannelMask(&channelMask);

            X3DAudioInitialize(channelMask, SPEED_OF_SOUND, x3d);

            return true;
        }

        void Shutdown()
        {
            if (master) { master->DestroyVoice(); master = nullptr; }
            xaudio.Reset();
        }
    };

    struct AudioListener
    {
        X3DAUDIO_LISTENER l{};

        void Set(const DirectX::XMFLOAT3& pos,
            const DirectX::XMFLOAT3& forward,
            const DirectX::XMFLOAT3& up,
            const DirectX::XMFLOAT3& velocity)
        {
            l.Position = { pos.x, pos.y, pos.z };
            l.OrientFront = { -forward.x, -forward.y, -forward.z };
            l.OrientTop = { up.x, up.y, up.z };
            l.Velocity = { velocity.x, velocity.y, velocity.z };
        }
    };

    static float DegToRad(float deg) { return deg * (X3DAUDIO_PI / 180.0f); }

    struct AudioEmitter
    {
        X3DAUDIO_EMITTER e{};
        std::vector<float> matrixCoeffs;
        std::vector<float> channelAzimuths;

        void InitPointEmitter(int srcChannels, int dstChannels)
        {
            e = X3DAUDIO_EMITTER{};
            e.ChannelCount = srcChannels;

            channelAzimuths.resize(srcChannels, 0.0f);

            if (srcChannels == 1)
            {
                channelAzimuths[0] = 0.0f;
            }
            else if (srcChannels == 2)
            {
                channelAzimuths[0] = -X3DAUDIO_PI / 2.0f;   // Left
                channelAzimuths[1] = X3DAUDIO_PI / 2.0f;    // Right
            }
            else if (srcChannels == 6) // 5.1
            {
                channelAzimuths[0] = DegToRad(-30);         // L
                channelAzimuths[1] = DegToRad(+30);         // R
                channelAzimuths[2] = DegToRad(0);           // C
                channelAzimuths[3] = DegToRad(0);           // LFE
                channelAzimuths[4] = DegToRad(-110);        // SL
                channelAzimuths[5] = DegToRad(+110);        // SR
            }
            else if (srcChannels == 8) // 7.1
            {
                channelAzimuths[0] = DegToRad(-30);         // L
                channelAzimuths[1] = DegToRad(+30);         // R
                channelAzimuths[2] = DegToRad(0);           // C
                channelAzimuths[3] = DegToRad(0);           // LFE
                channelAzimuths[4] = DegToRad(-90);         // SL
                channelAzimuths[5] = DegToRad(+90);         // SR
                channelAzimuths[6] = DegToRad(-150);        // BL
                channelAzimuths[7] = DegToRad(+150);        // BR
            }
            else 
            {
                for (int i = 0; i < srcChannels; ++i)
                    channelAzimuths[i] = (2.0f * X3DAUDIO_PI) * (float)i / (float)srcChannels;
            }

            e.pChannelAzimuths = channelAzimuths.data();

            e.CurveDistanceScaler = 1.0f;
            e.DopplerScaler = 1.0f;
            e.InnerRadius = 0.0f;
            e.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;

            matrixCoeffs.resize(srcChannels * dstChannels);
        }

        void Set(const DirectX::XMFLOAT3& pos,
            const DirectX::XMFLOAT3& forward,
            const DirectX::XMFLOAT3& up,
            const DirectX::XMFLOAT3& velocity)
        {
            e.Position = { pos.x, pos.y, pos.z };
            e.OrientFront = { forward.x, forward.y, forward.z };
            e.OrientTop = { up.x, up.y, up.z };
            e.Velocity = { velocity.x, velocity.y, velocity.z };
        }
    };

    struct Audio3DParams
    {
        X3DAUDIO_DSP_SETTINGS dsp{};
        float doppler = 1.0f;
    };

    inline Audio3DParams Calculate3D(AudioSystem& sys, AudioListener& L, AudioEmitter& E)
    {
        Audio3DParams out{};
        out.dsp.SrcChannelCount = E.e.ChannelCount;
        out.dsp.DstChannelCount = sys.dstChannels;
        out.dsp.pMatrixCoefficients = E.matrixCoeffs.data();

        const DWORD flags =
            X3DAUDIO_CALCULATE_MATRIX |
            X3DAUDIO_CALCULATE_DOPPLER;
            // + X3DAUDIO_CALCULATE_LPF_DIRECT / REVERB / DELAY è ò.ä.

        X3DAudioCalculate(sys.x3d, &L.l, &E.e, flags, &out.dsp);

        out.doppler = out.dsp.DopplerFactor;
        return out;
    }

    inline void Apply3D(IXAudio2SourceVoice* voice, AudioSystem& sys, AudioEmitter& E, const Audio3DParams& p)
    {
        voice->SetOutputMatrix(sys.master, E.e.ChannelCount, sys.dstChannels, E.matrixCoeffs.data());
        voice->SetFrequencyRatio(p.doppler);
    }


    static uint32_t ReadU32(std::ifstream& f)
    {
        uint32_t v; f.read(reinterpret_cast<char*>(&v), 4);
        if (!f) throw std::runtime_error("read u32 failed");
        return v;
    }
    static uint16_t ReadU16(std::ifstream& f)
    {
        uint16_t v; f.read(reinterpret_cast<char*>(&v), 2);
        if (!f) throw std::runtime_error("read u16 failed");
        return v;
    }

    struct WavData
    {
        WAVEFORMATEX wfx{};
        std::vector<uint8_t> audio;
    };

    inline WavData LoadWavRiff(const std::wstring& path)
    {
        std::ifstream f(path, std::ios::binary);
        if (!f) throw std::runtime_error("cannot open wav");

        auto fourcc = [&]() 
            {
            uint32_t v = ReadU32(f);
            return v;
            };

        const uint32_t RIFF = 'FFIR';
        const uint32_t WAVE = 'EVAW';
        const uint32_t FMT = ' tmf';
        const uint32_t DATA = 'atad';

        if (fourcc() != RIFF) throw std::runtime_error("not RIFF");
        (void)ReadU32(f);
        if (fourcc() != WAVE) throw std::runtime_error("not WAVE");

        WavData out{};
        bool gotFmt = false, gotData = false;

        while (f && (!gotFmt || !gotData))
        {
            uint32_t chunkId = fourcc();
            uint32_t chunkSize = ReadU32(f);

            if (chunkId == FMT)
            {
                if (chunkSize < 16) throw std::runtime_error("bad fmt chunk");

                WAVEFORMATEX wfx{};
                wfx.wFormatTag = ReadU16(f);
                wfx.nChannels = ReadU16(f);
                wfx.nSamplesPerSec = ReadU32(f);
                wfx.nAvgBytesPerSec = ReadU32(f);
                wfx.nBlockAlign = ReadU16(f);
                wfx.wBitsPerSample = ReadU16(f);

                uint16_t cbSize = 0;
                if (chunkSize > 16)
                {
                    cbSize = ReadU16(f);
                    if (chunkSize > 18)
                        f.seekg(static_cast<std::streamoff>(chunkSize - 18), std::ios::cur);
                }

                wfx.cbSize = cbSize;
                out.wfx = wfx;
                gotFmt = true;
            }
            else if (chunkId == DATA)
            {
                out.audio.resize(chunkSize);
                f.read(reinterpret_cast<char*>(out.audio.data()), chunkSize);
                if (!f) throw std::runtime_error("read data failed");
                gotData = true;
            }
            else
            {
                f.seekg(static_cast<std::streamoff>(chunkSize), std::ios::cur);
            }

            if (chunkSize & 1) f.seekg(1, std::ios::cur);
        }

        if (!gotFmt || !gotData) throw std::runtime_error("missing fmt or data");

        if (out.wfx.wFormatTag != WAVE_FORMAT_PCM &&
            out.wfx.wFormatTag != WAVE_FORMAT_IEEE_FLOAT &&
            out.wfx.wFormatTag != WAVE_FORMAT_EXTENSIBLE)
        {
            throw std::runtime_error("unsupported wav format tag");
        }

        return out;
    }

    struct PlayingSound
    {
        IXAudio2SourceVoice* voice = nullptr;
        WavData wav;
    };

    inline PlayingSound PlayWavOneShot(IXAudio2* xaudio, const WavData& wav)
    {
        PlayingSound s{};
        s.wav = wav;

        HRESULT hr = xaudio->CreateSourceVoice(&s.voice, &s.wav.wfx);
        if (FAILED(hr)) throw std::runtime_error("CreateSourceVoice failed");

        XAUDIO2_BUFFER buf{};
        buf.AudioBytes = (UINT32)s.wav.audio.size();
        buf.pAudioData = s.wav.audio.data();
        buf.Flags = XAUDIO2_END_OF_STREAM;

        hr = s.voice->SubmitSourceBuffer(&buf);
        if (FAILED(hr)) throw std::runtime_error("SubmitSourceBuffer failed");

        hr = s.voice->Start(0);
        if (FAILED(hr)) throw std::runtime_error("Start failed");

        return s;
    }

    struct SpatialSound
    {
        PlayingSound sound;
        AudioEmitter emitter;
    };

    inline SpatialSound SpawnSpatialWav(AudioSystem& audio, const WavData& wav)
    {
        SpatialSound ss{};
        ss.sound = PlayWavOneShot(audio.xaudio.Get(), wav);
        ss.emitter.InitPointEmitter(wav.wfx.nChannels, audio.dstChannels);
        return ss;
    }

    inline void UpdateSpatialSound(AudioSystem& audio, AudioListener& listener, SpatialSound& s)
    {
        auto params = Calculate3D(audio, listener, s.emitter);
        Apply3D(s.sound.voice, audio, s.emitter, params);
    }

    inline bool IsFinished(IXAudio2SourceVoice* v)
    {
        XAUDIO2_VOICE_STATE st{};
        v->GetState(&st);
        return st.BuffersQueued == 0;
    }

    inline void DestroyPlaying(PlayingSound& s)
    {
        if (s.voice)
        {
            s.voice->Stop(0);
            s.voice->DestroyVoice();
            s.voice = nullptr;
        }
    }
};
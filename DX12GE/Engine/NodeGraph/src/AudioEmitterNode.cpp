#include "../../Base/Singleton.h"
#include "../../Graphics/ResourceStorage.h"

AudioEmitterNode::AudioEmitterNode() : Node3D(), m_AudioComponentId(- 1)
{
	m_Type = NODE_TYPE_AUDIO_EMITTER;

	m_EmitterData = X3DAUDIO_EMITTER{};
    m_EmitterData.Position = { 0.0f, 0.0f, 0.0f };
    m_EmitterData.OrientFront = { 0.0f, 0.0f, 1.0f };
    m_EmitterData.OrientTop = { 0.0f, 1.0f, 0.0f };
    m_EmitterData.Velocity = { 0.0f, 0.0f, 0.0f };

    m_BaseVolume = 1.0f;
    m_BasePitch = 1.0f;
    Loop = false;
    DopplerEffect = false;
    Ubiquitous = false;

    m_IsPlaying = false;

    Rename("AudioEmitterNode");
}

void AudioEmitterNode::OnUpdate(const double& deltaTime)
{
    if (m_Voice && IsFinished())
    {
        DestroyPlayingSound();
    }

    if (!m_Voice && Loop)
    {
        SpawnPlayingSound(true);
    }

    bool dirty = Transform.IsCacheDirty();
    Node3D::OnUpdate(deltaTime);
    if (dirty)
    {
        m_EmitterData.OrientTop = TransformComponent::CalculateUpVector(m_WorldDirectionCache);
    }

    AudioSystem* system = Singleton::GetAudioSystem();
    AudioListenerNode* listener = Singleton::GetNodeGraph()->GetCurrentListener();

    if (Ubiquitous)
    {
        m_EmitterData.Position = listener->GetWorldPosition();
        m_EmitterData.OrientFront = { 0, 0, 0 };
    }
    else
    {
        m_EmitterData.Position = m_WorldPositionCache;
        m_EmitterData.OrientFront = m_WorldDirectionCache;
    }

    m_EmitterData.Velocity = (m_WorldPositionCache - m_PrevWorldPosition) / static_cast<float>(deltaTime);
    m_PrevWorldPosition = m_WorldPositionCache;

    if (!m_Voice || IsFinished())
    {
        m_IsPlaying = false;
        return;
    }

    // Calculate3D

    X3DAUDIO_DSP_SETTINGS dsp{};

    dsp.SrcChannelCount = m_EmitterData.ChannelCount;
    dsp.DstChannelCount = system->dstChannels;
    dsp.pMatrixCoefficients = m_MatrixCoeffs.data();
    dsp.EmitterToListenerDistance = 0.0f;

    const DWORD flags =
        X3DAUDIO_CALCULATE_MATRIX |
        X3DAUDIO_CALCULATE_DOPPLER; // + X3DAUDIO_CALCULATE_LPF_DIRECT / REVERB / DELAY
    
    X3DAudioCalculate(system->x3d, &listener->GetListenerData(), &m_EmitterData, flags, &dsp);

    // Apply3D

    m_Voice->SetVolume(m_BaseVolume);

    if (!DopplerEffect)
    {
        dsp.DopplerFactor = 1.0f;
    }

    m_Voice->SetOutputMatrix(system->master, m_EmitterData.ChannelCount, system->dstChannels, m_MatrixCoeffs.data());

    float freq = static_cast<float>(dsp.DopplerFactor) * m_BasePitch;
    freq = std::clamp(freq, XAUDIO2_MIN_FREQ_RATIO, XAUDIO2_MAX_FREQ_RATIO);
    m_Voice->SetFrequencyRatio(freq);
}

bool AudioEmitterNode::StartPlayingSound()
{
    if (!m_Voice || FAILED(m_Voice->Start(0)))
    {
        printf("Error! Audio start failed.\n");
        return false;
    }
    m_IsPlaying = true;
    return true;
}

bool AudioEmitterNode::StopPlayingSound()
{
    if (!m_Voice || FAILED(m_Voice->Stop(0)))
    {
        return false;
    }
    m_IsPlaying = false;
    return true;
}

bool AudioEmitterNode::SpawnPlayingSound(bool startPlay)
{
    DestroyPlayingSound();

    AudioSystem* system = Singleton::GetAudioSystem();

    if (!IsValid())
        return false;

    auto wavComponent = ResourceStorage::GetAudio(m_AudioComponentId);

    if (!wavComponent || !wavComponent->IsInitialized)
        return false;

    HRESULT hr = system->xaudio->CreateSourceVoice(&m_Voice, &wavComponent->WavWFX);
    if (FAILED(hr))
        return false;

    XAUDIO2_BUFFER buf{};
    buf.AudioBytes = (UINT32)wavComponent->WavAudioData.size();
    buf.pAudioData = wavComponent->WavAudioData.data();
    buf.Flags = XAUDIO2_END_OF_STREAM;

    hr = m_Voice->SubmitSourceBuffer(&buf);
    if (FAILED(hr))
        return false;

    if (startPlay)
    {
        if (!StartPlayingSound())
            return false;
    }

    InitPointEmitter(wavComponent->WavWFX.nChannels, system->dstChannels);

    return true;
}

bool AudioEmitterNode::DestroyPlayingSound()
{
    if (m_Voice)
    {
        if (!IsFinished())
        {
            StopPlayingSound();
        }
        m_Voice->DestroyVoice();
        m_Voice = nullptr;
        return true;
    }
    return false;
}

bool AudioEmitterNode::IsFinished()
{
    if (!m_Voice) return false;

    XAUDIO2_VOICE_STATE st{};
    m_Voice->GetState(&st);
    return st.BuffersQueued == 0;
}

void AudioEmitterNode::SetVolume(float vol)
{
    if (vol >= 0.0f && vol <= 32.0f)
    {
        m_BaseVolume = vol;
    }
}

float AudioEmitterNode::GetVolume()
{
    return m_BaseVolume;
}

void AudioEmitterNode::SetPitch(float pitch)
{
    if (pitch >= 0.0f && pitch <= 32.0f)
    {
        m_BasePitch = pitch;
    }
}

float AudioEmitterNode::GetPitch()
{
    return m_BasePitch;
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

void AudioEmitterNode::LoadWav(const std::string& path)
{
    int id = ResourceStorage::AddAudio(path);
    auto wavComponent = ResourceStorage::GetAudio(id);

    if (wavComponent->IsInitialized)
    {
        m_AudioComponentId = id;
        return;
    }

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
            wavComponent->WavWFX = wfx;
            gotFmt = true;
        }
        else if (chunkId == DATA)
        {
            wavComponent->WavAudioData.resize(chunkSize);
            f.read(reinterpret_cast<char*>(wavComponent->WavAudioData.data()), chunkSize);
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

    if (wavComponent->WavWFX.wFormatTag != WAVE_FORMAT_PCM &&
        wavComponent->WavWFX.wFormatTag != WAVE_FORMAT_IEEE_FLOAT &&
        wavComponent->WavWFX.wFormatTag != WAVE_FORMAT_EXTENSIBLE)
    {
        throw std::runtime_error("unsupported wav format tag");
    }

    wavComponent->FilePath = path;
    wavComponent->IsInitialized = true;
    m_AudioComponentId = id;
}

void AudioEmitterNode::InitPointEmitter(int srcChannels, int dstChannels)
{
    m_EmitterData = X3DAUDIO_EMITTER{};
    std::vector<float> channelAzimuths;

    m_EmitterData.ChannelCount = srcChannels;

    channelAzimuths.resize(srcChannels, 0.0f);

    if (srcChannels == 1)
    {
        channelAzimuths[0] = 0.0f;
    }
    else if (srcChannels == 2)
    {
        channelAzimuths[0] = -X3DAUDIO_PI / 2.0f;       // Left
        channelAzimuths[1] = X3DAUDIO_PI / 2.0f;        // Right
    }
    else if (srcChannels == 6)  // 5.1
    {
        channelAzimuths[0] = Math::ToRadians(-30.0f);   // L
        channelAzimuths[1] = Math::ToRadians(+30.0f);   // R
        channelAzimuths[2] = Math::ToRadians(0.0f);     // C
        channelAzimuths[3] = Math::ToRadians(0.0f);     // LFE
        channelAzimuths[4] = Math::ToRadians(-110.0f);  // SL
        channelAzimuths[5] = Math::ToRadians(+110.0f);  // SR
    }
    else if (srcChannels == 8)  // 7.1
    {
        channelAzimuths[0] = Math::ToRadians(-30.0f);   // L
        channelAzimuths[1] = Math::ToRadians(+30.0f);   // R
        channelAzimuths[2] = Math::ToRadians(0.0f);     // C
        channelAzimuths[3] = Math::ToRadians(0.0f);     // LFE
        channelAzimuths[4] = Math::ToRadians(-90.0f);   // SL
        channelAzimuths[5] = Math::ToRadians(+90.0f);   // SR
        channelAzimuths[6] = Math::ToRadians(-150.0f);  // BL
        channelAzimuths[7] = Math::ToRadians(+150.0f);  // BR
    }
    else
    {
        for (int i = 0; i < srcChannels; ++i)
            channelAzimuths[i] = (2.0f * X3DAUDIO_PI) * (float)i / (float)srcChannels;
    }

    m_EmitterData.pChannelAzimuths = channelAzimuths.data();
    m_EmitterData.CurveDistanceScaler = 1.0f;
    m_EmitterData.DopplerScaler = 1.0f;
    m_EmitterData.InnerRadius = 0.0f;
    m_EmitterData.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;

    m_MatrixCoeffs.resize(srcChannels * dstChannels);
}

void AudioEmitterNode::SetComponentId(uint32_t newId)
{
    if (newId < 0 || newId >= ResourceStorage::AudioCount())
    {
        printf("Error: Component ID of audio is outside the array size in ResourceStorage\n");
        return;
    }
    m_AudioComponentId = newId;
}

const std::string AudioEmitterNode::GetWavFilePath()
{
    if (!IsValid()) return "";
    return ResourceStorage::GetAudio(m_AudioComponentId)->FilePath;
}

void AudioEmitterNode::Destroy(bool keepComponent)
{
    if (m_AudioComponentId != -1 && !(keepComponent || TreeHasEmittersWithComponentId(m_AudioComponentId)))
    {
        ResourceStorage::DeleteAudioComponentForever(m_AudioComponentId);
    }
    m_AudioComponentId = -1;

    DestroyPlayingSound();

    Node3D::Destroy();
}

Node3D* AudioEmitterNode::Clone(Node3D* newParent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
    if (!cloneNode)
    {
        cloneNode = new AudioEmitterNode();
    }

    Node3D::Clone(newParent, cloneChildrenRecursive, cloneNode);

    if (AudioEmitterNode* emit = dynamic_cast<AudioEmitterNode*>(cloneNode))
    {
        emit->SetComponentId(GetComponentId());
        emit->m_BaseVolume = m_BaseVolume;
        emit->m_BasePitch = m_BasePitch;
        emit->Loop = Loop;
        emit->DopplerEffect = DopplerEffect;
    }

    return cloneNode;
}

bool AudioEmitterNode::TreeHasEmittersWithComponentId(uint32_t id, Node3D* current)
{
    current = current == nullptr ? Singleton::GetNodeGraph()->GetRoot() : current;

    if (AudioEmitterNode* emit = dynamic_cast<AudioEmitterNode*>(current))
    {
        if (emit->GetComponentId() == id && emit != this)
            return true;
    }

    for (auto child : current->GetChildren())
    {
        if (TreeHasEmittersWithComponentId(id, child))
            return true;
    }

    return false;
}

void AudioEmitterNode::DrawDebug()
{
    Node3D::DrawDebug();
    Singleton::GetDebugRender()->DrawSphere(0.2, Color(1, 1, 1), m_WorldMatrixCache, 8);
}

void AudioEmitterNode::CreateJsonData(json& j)
{
    Node3D::CreateJsonData(j);

    j["file_path"] = GetWavFilePath();
    j["audio_volume"] = m_BaseVolume;
    j["audio_pitch"] = m_BasePitch;
    j["audio_loop"] = Loop;
    j["audio_doppler"] = DopplerEffect;
    j["audio_ubiquitous"] = Ubiquitous;
}

void AudioEmitterNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
    Node3D::LoadFromJsonData(nodeData);

    m_BaseVolume = nodeData.audioVolume;
    m_BasePitch = nodeData.audioPitch;
    Loop = nodeData.audioLoop;
    DopplerEffect = nodeData.audioDoppler;
    Ubiquitous = nodeData.audioUbiquitous;
}

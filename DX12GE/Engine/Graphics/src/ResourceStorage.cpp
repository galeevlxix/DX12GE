#include "../ResourceStorage.h"
#include <string>
#include <map>
#include <vector>

static std::map<std::string, uint32_t> m_Names;
static std::vector<std::shared_ptr<Object3DComponent>> m_Objects;
static std::vector<std::shared_ptr<TextureComponent>> m_Textures;
static std::vector<std::shared_ptr<AudioWavComponent>> m_Audio;

uint32_t ResourceStorage::Find(const std::string& name)
{
    auto pair = m_Names.find(name);
    return pair != m_Names.end() ? pair->second : -1;
}

uint32_t ResourceStorage::AddObject3D(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1) return foundId;

    uint32_t id = static_cast<uint32_t>(m_Objects.size());
    m_Objects.push_back(std::make_shared<Object3DComponent>());
    m_Names.emplace(name, id);

    return id;
}

uint32_t ResourceStorage::AddTexture(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1) 
        return foundId;

    uint32_t id = static_cast<uint32_t>(m_Textures.size());
    m_Textures.push_back(std::make_shared<TextureComponent>());
    m_Names.emplace(name, id);

	return id;
}

uint32_t ResourceStorage::AddAudio(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1)
        return foundId;

    uint32_t id = static_cast<uint32_t>(m_Audio.size());
    m_Audio.push_back(std::make_shared<AudioWavComponent>());
    m_Names.emplace(name, id);

    return id;
}

std::shared_ptr<Object3DComponent> ResourceStorage::GetObject3D(uint32_t id)
{
    if (id < 0 || id >= m_Objects.size()) 
        return nullptr;

    return m_Objects[id];
}

std::shared_ptr<TextureComponent> ResourceStorage::GetTexture(uint32_t id)
{
    if (id < 0 || id >= m_Textures.size())
        return nullptr;

    return m_Textures[id];
}

std::shared_ptr<AudioWavComponent> ResourceStorage::GetAudio(uint32_t id)
{
    if (id < 0 || id >= m_Audio.size())
        return nullptr;

    return m_Audio[id];
}

std::shared_ptr<Object3DComponent> ResourceStorage::GetObject3DByName(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId >= 0 && foundId < m_Objects.size())
        return m_Objects[foundId];
    return nullptr;
}

std::shared_ptr<TextureComponent> ResourceStorage::GetTextureByName(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId >= 0 && foundId < m_Textures.size())
        return m_Textures[foundId];
    return nullptr;
}

std::shared_ptr<AudioWavComponent> ResourceStorage::GetAudioByName(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId >= 0 && foundId < m_Audio.size())
        return m_Audio[foundId];
    return nullptr;
}

void ResourceStorage::DeleteObject3DComponentForever(uint32_t id)
{
    if (id < 0 || id >= m_Objects.size())
        return;

    if (!m_Objects[id]) return;

    m_Objects[id]->Destroy();
    m_Objects[id].reset();
    m_Objects[id] = nullptr;
}

void ResourceStorage::DeleteTextureComponentForever(uint32_t id)
{
    if (id < 0 || id >= m_Textures.size())
        return;

    if (!m_Textures[id]) return;

    m_Textures[id]->Destroy();
    m_Textures[id].reset();
    m_Textures[id] = nullptr;
}

void ResourceStorage::DeleteAudioComponentForever(uint32_t id)
{
    if (id < 0 || id >= m_Audio.size())
        return;

    if (!m_Audio[id]) return;

    m_Audio[id]->Destroy();
    m_Audio[id].reset();
    m_Audio[id] = nullptr;
}

size_t ResourceStorage::ObjectsCount()
{
    return m_Objects.size();
}

size_t ResourceStorage::TexturesCount()
{
    return m_Textures.size();
}

size_t ResourceStorage::AudioCount()
{
    return m_Audio.size();
}

void ResourceStorage::Destroy()
{
    for (auto obj : m_Objects)
    {
        obj->Destroy();
        obj.reset();
        obj = nullptr;
    }
    m_Objects.clear();

    for (auto tex : m_Textures)
    {
        tex->Destroy();
        tex.reset();
        tex = nullptr;
    }
    m_Textures.clear();

    for (auto audio : m_Audio)
    {
        audio->Destroy();
        audio.reset();
        audio = nullptr;
    }
    m_Audio.clear();

    m_Names.clear();
}

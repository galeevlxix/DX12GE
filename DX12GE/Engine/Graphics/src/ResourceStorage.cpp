#include "../ResourceStorage.h"
#include <string>
#include <map>
#include <vector>
#include <algorithm>

static std::map<std::string, uint32_t> m_Names;
static std::vector<std::shared_ptr<Object3DComponent>> m_Objects;
static std::vector<std::shared_ptr<TextureComponent>> m_Textures;
static std::vector<std::shared_ptr<AudioWavComponent>> m_Audio;

static std::vector<PointLightComponent> m_PointLightComponents;
static std::vector<uint32_t> m_PointLightFreeIndices;

static std::vector<SpotLightComponent> m_SpotLightComponents;
static std::vector<uint32_t> m_SpotLightFreeIndices;

bool ArrayContains(std::vector<uint32_t>& array, const uint32_t& element)
{
    return std::binary_search(array.begin(), array.end(), element);
}

void InsertSorted(std::vector<uint32_t>& array, const uint32_t& element)
{
    auto it = std::lower_bound(array.begin(), array.end(), element);
    array.insert(it, element);
}

uint32_t ResourceStorage::Find(const std::string& name)
{
    auto pair = m_Names.find(name);
    return pair != m_Names.end() ? pair->second : -1;
}

uint32_t ResourceStorage::AddObject3D(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1 && GetObject3D(foundId) != nullptr)
        return foundId;

    uint32_t id = static_cast<uint32_t>(m_Objects.size());
    m_Objects.push_back(std::make_shared<Object3DComponent>());
    m_Names.emplace(name, id);

    return id;
}

uint32_t ResourceStorage::AddTexture(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1 && GetTexture(foundId) != nullptr)
        return foundId;

    uint32_t id = static_cast<uint32_t>(m_Textures.size());
    m_Textures.push_back(std::make_shared<TextureComponent>());
    m_Names.emplace(name, id);

	return id;
}

uint32_t ResourceStorage::AddAudio(const std::string& name)
{
    uint32_t foundId = Find(name);
    if (foundId != -1 && GetAudio(foundId) != nullptr)
        return foundId;

    uint32_t id = static_cast<uint32_t>(m_Audio.size());
    m_Audio.push_back(std::make_shared<AudioWavComponent>());
    m_Names.emplace(name, id);

    return id;
}

uint32_t ResourceStorage::AddPointLightComponent()
{
    uint32_t id;

    if (!m_PointLightFreeIndices.empty())
    {
        id = m_PointLightFreeIndices.back();
        m_PointLightComponents[id] = PointLightComponent();
        m_PointLightFreeIndices.pop_back();
    }
    else
    {
        id = static_cast<uint32_t>(m_PointLightComponents.size());
        m_PointLightComponents.emplace_back();
    }

    return id;
}

uint32_t ResourceStorage::AddSpotLightComponent()
{
    uint32_t id;

    if (!m_SpotLightFreeIndices.empty())
    {
        id = m_SpotLightFreeIndices.back();
        m_SpotLightComponents[id] = SpotLightComponent();
        m_SpotLightFreeIndices.pop_back();
    }
    else
    {
        id = static_cast<uint32_t>(m_SpotLightComponents.size());
        m_SpotLightComponents.emplace_back();
    }

    return id;
}

std::shared_ptr<Object3DComponent> ResourceStorage::GetObject3D(const uint32_t& id)
{
    if (id < 0 || id >= m_Objects.size()) 
        return nullptr;

    return m_Objects[id];
}

std::shared_ptr<TextureComponent> ResourceStorage::GetTexture(const uint32_t& id)
{
    if (id < 0 || id >= m_Textures.size())
        return nullptr;

    return m_Textures[id];
}

std::shared_ptr<AudioWavComponent> ResourceStorage::GetAudio(const uint32_t& id)
{
    if (id < 0 || id >= m_Audio.size())
        return nullptr;

    return m_Audio[id];
}

PointLightComponent* ResourceStorage::GetPointLightComponent(const uint32_t& id)
{
    if (id < 0 || id >= m_PointLightComponents.size() || ArrayContains(m_PointLightFreeIndices, id))
        return nullptr;

    return &m_PointLightComponents[id];
}

SpotLightComponent* ResourceStorage::GetSpotLightComponent(const uint32_t& id)
{
    if (id < 0 || id >= m_SpotLightComponents.size() || ArrayContains(m_SpotLightFreeIndices, id))
        return nullptr;

    return &m_SpotLightComponents[id];
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

void ResourceStorage::DeleteObject3DComponentForever(const uint32_t& id)
{
    if (id < 0 || id >= m_Objects.size())
        return;

    if (!m_Objects[id]) return;

    m_Objects[id]->Destroy();
    m_Objects[id].reset();
    m_Objects[id] = nullptr;
}

void ResourceStorage::DeleteTextureComponentForever(const uint32_t& id)
{
    if (id < 0 || id >= m_Textures.size())
        return;

    if (!m_Textures[id]) return;

    m_Textures[id]->Destroy();
    m_Textures[id].reset();
    m_Textures[id] = nullptr;
}

void ResourceStorage::DeleteAudioComponentForever(const uint32_t& id)
{
    if (id < 0 || id >= m_Audio.size())
        return;

    if (!m_Audio[id]) return;

    m_Audio[id]->Destroy();
    m_Audio[id].reset();
    m_Audio[id] = nullptr;
}

void ResourceStorage::DeletePointLightComponentForever(const uint32_t& id)
{
    if (id < 0 || id >= m_PointLightComponents.size() || ArrayContains(m_PointLightFreeIndices, id))
        return;

    m_PointLightComponents[id].BaseLightProperties.Intensity = 0.0f;
    InsertSorted(m_PointLightFreeIndices, id);
}

void ResourceStorage::DeleteSpotLightComponentForever(const uint32_t& id)
{
    if (id < 0 || id >= m_SpotLightComponents.size() || ArrayContains(m_SpotLightFreeIndices, id))
        return;

    m_SpotLightComponents[id].PointLightProperties.BaseLightProperties.Intensity = 0.0f;
    InsertSorted(m_SpotLightFreeIndices, id);
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

size_t ResourceStorage::PointLightComponentsCount()
{
    return m_PointLightComponents.size();
}

size_t ResourceStorage::SpotLightComponentsCount()
{
    return m_SpotLightComponents.size();
}

std::vector<PointLightComponent>* ResourceStorage::GetPointLightComponentsArray()
{
    return &m_PointLightComponents;
}

std::vector<SpotLightComponent>* ResourceStorage::GetSpotLightComponentsArray()
{
    return &m_SpotLightComponents;
}

void ResourceStorage::Destroy()
{
    for (auto obj : m_Objects)
    {
		if (!obj) continue;
        obj->Destroy();
        obj.reset();
        obj = nullptr;
    }
    m_Objects.clear();

    for (auto tex : m_Textures)
    {
        if (!tex) continue;
        tex->Destroy();
        tex.reset();
        tex = nullptr;
    }
    m_Textures.clear();

    for (auto audio : m_Audio)
    {
        if (!audio) continue;
        audio->Destroy();
        audio.reset();
        audio = nullptr;
    }
    m_Audio.clear();

    m_Names.clear();

    m_PointLightComponents.clear();
    m_PointLightFreeIndices.clear();
    m_SpotLightComponents.clear();
    m_SpotLightFreeIndices.clear();
}

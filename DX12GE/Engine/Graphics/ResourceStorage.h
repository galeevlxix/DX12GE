#pragma once
#include "Object3DComponent.h"
#include "TextureComponent.h"
#include "../Base/AudioWavComponent.h"
#include "GraphicsComponents.h"

class ResourceStorage
{
public:

	static uint32_t Find(const std::string& name);
	static uint32_t AddObject3D(const std::string& name);
	static uint32_t AddTexture(const std::string& name);
	static uint32_t AddAudio(const std::string& name);
	static uint32_t AddPointLightComponent();
	static uint32_t AddSpotLightComponent();
	
	static std::shared_ptr<Object3DComponent> GetObject3D(const uint32_t& id);
	static std::shared_ptr<TextureComponent> GetTexture(const uint32_t& id);
	static std::shared_ptr<AudioWavComponent> GetAudio(const uint32_t& id);
	static PointLightComponent* GetPointLightComponent(const uint32_t& id);
	static SpotLightComponent* GetSpotLightComponent(const uint32_t& id);

	static std::shared_ptr<Object3DComponent> GetObject3DByName(const std::string& name);
	static std::shared_ptr<TextureComponent> GetTextureByName(const std::string& name);
	static std::shared_ptr<AudioWavComponent> GetAudioByName(const std::string& name);

	static void DeleteObject3DComponentForever(const uint32_t& id);
	static void DeleteTextureComponentForever(const uint32_t& id);
	static void DeleteAudioComponentForever(const uint32_t& id);
	static void DeletePointLightComponentForever(const uint32_t& id);
	static void DeleteSpotLightComponentForever(const uint32_t& id);

	static size_t ObjectsCount();
	static size_t TexturesCount();
	static size_t AudioCount();
	static size_t PointLightComponentsCount();
	static size_t SpotLightComponentsCount();

	static std::vector<PointLightComponent>* GetPointLightComponentsArray();
	static std::vector<SpotLightComponent>* GetSpotLightComponentsArray();

	static void Destroy();
};

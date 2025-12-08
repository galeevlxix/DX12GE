#pragma once
#include "Object3DComponent.h"
#include "TextureComponent.h"

class ResourceStorage
{
public:
	static uint32_t Find(const std::string& name);
	static uint32_t AddObject3D(const std::string& name);
	static uint32_t AddTexture(const std::string& name);
	
	static std::shared_ptr<Object3DComponent> GetObject3D(uint32_t id);
	static std::shared_ptr<TextureComponent> GetTexture(uint32_t id);
	static std::shared_ptr<Object3DComponent> GetObject3DByName(const std::string& name);
	static std::shared_ptr<TextureComponent> GetTextureByName(const std::string& name);

	static void DeleteObject3DComponentForever(uint32_t id);
	static void DeleteTextureComponentForever(uint32_t id);

	static size_t ObjectsCount();
	static size_t TexturesCount();

	static void Destroy();
};

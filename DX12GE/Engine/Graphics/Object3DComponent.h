#pragma once

#include "Mesh3DComponent.h"
#include <vector>

class Object3DComponent
{
private:
	std::vector<Mesh3DComponent*> m_Meshes;
	bool m_Initialized = false;
public:
	std::string ResourcePath;
	void OnLoad(std::vector<Mesh3DComponent*>& meshes);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList);
	bool IsInitialized();
	void Destroy();
};
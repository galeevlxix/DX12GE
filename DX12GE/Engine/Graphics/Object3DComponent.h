#pragma once

#include "Mesh3DComponent.h"
#include "../Base/CollisionBox.h"
#include <vector>

class Object3DComponent
{
private:
	std::vector<Mesh3DComponent*> m_Meshes;
	std::vector<Vector3> m_Vertices;
	
	bool m_Initialized = false;
public:
	std::string ResourcePath;
	CollisionBox Box;

	void OnLoad(std::vector<Mesh3DComponent*>& meshes, std::vector<Vector3>* modelVertices);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList);
	std::vector<Vector3>* GetVertices() { return &m_Vertices; };
	bool IsInitialized();
	void Destroy();
};
#pragma once
#include "../Base/DX12LibPCH.h"
#include "Mesh3DComponent.h"
#include "../Graphics/VertexStructures.h"
#include <string>
#include <vector>

class AssimpModelLoader
{
public:
	uint32_t LoadModelData(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string &filePath, const std::string& nodePath, float& OutYOffset, vector<Vector3>* OutVertices = nullptr);
};
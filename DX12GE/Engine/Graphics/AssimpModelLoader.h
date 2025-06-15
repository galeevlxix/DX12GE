#pragma once
#include "../Base/DX12LibPCH.h"
#include "Mesh3DComponent.h"
#include <string>
#include <vector>

class AssimpModelLoader
{
public:
	uint32_t LoadModelData(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string &filePath, float& OutYOffset);

private:
};
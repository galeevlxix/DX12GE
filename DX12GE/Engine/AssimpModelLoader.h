#pragma once

#include <string>
#include "DX12LibPCH.h"

#include "../Game/BaseObject.h"
#include "Material.h"

using namespace std;

class AssimpModelLoader
{
public:
	bool LoadModelData(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath, vector<BaseObject> *Meshes, vector<Material> *Materials, vector<int> *MaterialIndices, float* OutYOffset);

private:
	int meshesCount;
	int materialsCount;

	string directory;
};
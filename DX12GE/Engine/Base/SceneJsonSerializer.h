#pragma once
#include "DX12LibPCH.h"

class SceneJsonSerializer
{
public:
	SceneJsonSerializer() {};

	void Save();
	void Load(ComPtr<ID3D12GraphicsCommandList2> commandList);
};
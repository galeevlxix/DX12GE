#pragma once

#include "../Graphics/Object3DEntity.h"
#include <string>
#include <map>

class SceneJsonSerializer
{
public:
	static void Save(std::map<std::string, Object3DEntity*>& objects);
	static void Load(ComPtr<ID3D12GraphicsCommandList2> commandList, std::map<std::string, Object3DEntity*>& objects);
};
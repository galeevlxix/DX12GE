#pragma once
#include "../Graphics/Object3DEntity.h"
#include <string>
#include <vector>
#include <map>

class JsonScene
{
private:
	const std::string path = "../../DX12GE/Resources/scene.json";
public:
	void Save(std::map<std::string, Object3DEntity>& objects);
	void Load(ComPtr<ID3D12GraphicsCommandList2> commandList, std::map<std::string, Object3DEntity>& objects);
};
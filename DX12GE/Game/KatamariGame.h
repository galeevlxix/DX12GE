#pragma once

#include "Player.h"
#include <vector>
#include <map>
#include <string>

using namespace std;

class KatamariGame
{
private:
	map<string, Object3DEntity> m_objects;

public:
	Player player;

	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void OnUpdate(float deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
private:
	void Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path);
};
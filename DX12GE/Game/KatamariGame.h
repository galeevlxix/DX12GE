#pragma once

#include "Player.h"
#include <vector>
#include <map>
#include <string>
#include "../Engine/Base/JsonScene.h"

using namespace std;

class KatamariGame
{
private:
	map<string, Object3DEntity> m_objects;

public:
	Player player;

	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnExit();
	void OnUpdate(float deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
	Object3DEntity* Get(std::string name);

	void Save();

private:
	JsonScene js;
	bool alwaysSave = false;
	void Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path);	
};
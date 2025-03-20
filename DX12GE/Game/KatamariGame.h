#pragma once

#include "Player.h"
//#include "BianObject.h"
#include <vector>
#include <map>
#include <string>

using namespace std;

class KatamariGame
{
private:
	vector<string> m_names;
	map<string, BianObject> m_objects;

	int fieldSize = 6;
	int cellSize = 12;
	
	int itemCount = 5;
public:
	Player player;

	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void CheckCollisions();

	void OnUpdate(float deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
private:
	void CreateField(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path);
	void Remove(string name);

};
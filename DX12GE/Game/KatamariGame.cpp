#include "KatamariGame.h"
#include "../Engine/ShaderResources.h"

int GetRandomNumber(int start, int end)
{
	return rand() % (end - start + 1) + start;
}

void KatamariGame::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	player.OnLoad(commandList);

	Add(commandList, "scene_forest", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene_forest"].SetScale(3, 3, 3);
}

void KatamariGame::OnUpdate(float deltaTime)
{
	player.OnUpdate(deltaTime);

	for (string name : m_names)
	{
		m_objects[name].OnUpdate(deltaTime);
	}
}

void KatamariGame::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	player.OnRender(commandList, viewProjMatrix);

	for (string name : m_names)
	{
		m_objects[name].OnRender(commandList, viewProjMatrix);
	}
}

void KatamariGame::CreateField(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	for (UINT i = 0; i < fieldSize; i++)
	{
		for (UINT j = 0; j < fieldSize; j++)
		{
			string name = "field" + to_string(i) + "_" + to_string(j);
			Add(commandList, name, "../../DX12GE/Resources/Katamari Objects/field/cube.obj");
			m_objects[name].SetScale(cellSize, 0.01f, cellSize);
			float offset = fieldSize % 2 == 1 ? cellSize : 0;
			m_objects[name].SetPosition((i - fieldSize * 0.5f) * cellSize * 2 + cellSize, -0.01f / 2.0f , (j - fieldSize * 0.5f) * cellSize * 2 + cellSize);
		}
	}
}

void KatamariGame::Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path)
{
	// KEY found
	if (m_objects.find(name) != m_objects.end()) { return; }

	m_names.push_back(name);
	m_objects.insert({ name, BianObject() });
	m_objects[name].OnLoad(commandList, path);
}

void KatamariGame::Remove(string name)
{
	
}

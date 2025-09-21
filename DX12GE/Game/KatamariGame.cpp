#include "KatamariGame.h"
#include "../Engine/Graphics/ShaderResources.h"

void KatamariGame::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	player.OnLoad(commandList);
	js.Load(commandList, m_objects);
}

void KatamariGame::OnUpdate(float deltaTime)
{
	player.OnUpdate(deltaTime);

	for (auto obj : m_objects)
	{
		obj.second.OnUpdate(deltaTime);
	}
}

void KatamariGame::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	player.OnRender(commandList, viewProjMatrix);

	for (auto obj : m_objects)
	{
		obj.second.OnRender(commandList, viewProjMatrix);
	}
}

Object3DEntity* KatamariGame::Get(std::string name)
{
	if (m_objects.find(name) == m_objects.end()) return nullptr;
	return &m_objects[name];
}

void KatamariGame::Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path)
{
	if (m_objects.find(name) != m_objects.end()) { return; }

	m_objects.insert({ name, Object3DEntity() });
	m_objects[name].OnLoad(commandList, path);
}

void KatamariGame::Save()
{
	js.Save(m_objects);
}

void KatamariGame::Exit()
{
	if (alwaysSave)
		Save();
}
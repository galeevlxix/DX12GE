#include "KatamariGame.h"
#include "../Engine/Graphics/ShaderResources.h"

void KatamariGame::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	player.OnLoad(commandList);

	js.Load(commandList, m_objects);
}

void KatamariGame::OnExit()
{
	js.Save(m_objects);
}

void KatamariGame::OnUpdate(float deltaTime)
{
	player.OnUpdate(deltaTime);

	static float counter = 0.0f;
	counter += deltaTime;

	m_objects["scene1"].Transform.Move(0, 5 * deltaTime, 0);
	m_objects["scene4"].Transform.SetRotationY(-counter);

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

void KatamariGame::Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path)
{
	if (m_objects.find(name) != m_objects.end()) { return; }

	m_objects.insert({ name, Object3DEntity() });
	m_objects[name].OnLoad(commandList, path);
}
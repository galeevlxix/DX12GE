#include "KatamariGame.h"
#include "../Engine/Graphics/ShaderResources.h"

void KatamariGame::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	player.OnLoad(commandList);

	Add(commandList, "scene", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene"].Transform.SetScale(3, 3, 3);
	m_objects["scene"].Transform.SetPosition(0, 0, 0);

	Add(commandList, "scene1", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene1"].Transform.SetScale(3, 3, 3);
	m_objects["scene1"].Transform.SetPosition(-75, 0, 0);

	Add(commandList, "scene2", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene2"].Transform.SetScale(3, 3, 3);
	m_objects["scene2"].Transform.SetPosition(-150, 0, 0);

	Add(commandList, "scene3", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene3"].Transform.SetScale(3, 3, 3);
	m_objects["scene3"].Transform.SetPosition(0, 0, 75);

	Add(commandList, "scene4", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene4"].Transform.SetScale(3, 3, 3);
	m_objects["scene4"].Transform.SetPosition(-75, 0, 75);

	Add(commandList, "scene5", "../../DX12GE/Resources/Models/gaz/scene.gltf");
	m_objects["scene5"].Transform.SetScale(3, 3, 3);
	m_objects["scene5"].Transform.SetPosition(-150, 0, 75);
}

void KatamariGame::OnUpdate(float deltaTime)
{
	player.OnUpdate(deltaTime);

	static float counter = 0.0f;
	counter += deltaTime;

	if (counter >= 2 * PI) counter -= 2 * PI;

	Vector3 sc1Pos = m_objects["scene1"].Transform.GetPosition();
	sc1Pos.y = sin(counter) * 10;
	m_objects["scene1"].Transform.SetPosition(sc1Pos);
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
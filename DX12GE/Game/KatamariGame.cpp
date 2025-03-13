#include "KatamariGame.h"

void KatamariGame::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	Add(commandList, "prince", "../../DX12GE/Resources/Katamari Objects/prince_katamari_damacy/gltf/scene.gltf");
	//"C:\Users\gtimu\source\repos\DX12GE\DX12GE\Resources\Katamari Objects\low-poly-boot\source\model.fbx"
	m_objects["prince"].SetRotationY(PI);

	CreateField(commandList);

	Add(commandList, "ball", "../../DX12GE/Resources/Katamari Objects/katamari_ball/core_01.obj");
	m_objects["ball"].Move(10, 2.2f, 0);
	m_objects["ball"].SetScale(100, 100, 100);

	Add(commandList, "boot", "../../DX12GE/Resources/Katamari Objects/low-poly-boot/source/model.fbx");
	m_objects["boot"].Move(20, 0, 0);
	m_objects["boot"].SetRotationY(PI);
	m_objects["boot"].SetScale(20, 20, 20);

	Add(commandList, "cup", "../../DX12GE/Resources/Katamari Objects/low-poly-cup-with-lemon-tea/source/Cup.fbx");
	m_objects["cup"].Move(30, 0, 0);
	m_objects["cup"].SetScale(0.3f, 0.3f, 0.3f);

	Add(commandList, "juice", "../../DX12GE/Resources/Katamari Objects/low-poly-stylized-juice/model.dae");
	m_objects["juice"].Move(40, 2, 0);
	m_objects["juice"].SetScale(150, 150, 150);

	Add(commandList, "chair", "../../DX12GE/Resources/Katamari Objects/old-wooden-chair-low-poly/chair.fbx");
	m_objects["chair"].Move(-10, 0, 0);
	m_objects["chair"].SetRotationY(PI);
	m_objects["chair"].SetScale(0.05, 0.05, 0.05);

	Add(commandList, "toothbrush", "../../DX12GE/Resources/Katamari Objects/toothbrush/model.dae");
	m_objects["toothbrush"].Move(-20, -100 + 2, 0);
	m_objects["toothbrush"].SetRotationY(PI);
	m_objects["toothbrush"].SetScale(0.03, 0.03, 0.03);
}

void KatamariGame::Start()
{
}

void KatamariGame::OnUpdate(float deltaTime)
{
	static Vector3 rotSpeed(PI / 2, 0, 0);
	m_objects["ball"].Rotate(rotSpeed * deltaTime);

	for (string name : m_names)
	{
		if (m_objects.find(name) == m_objects.end()) { continue; }
		m_objects[name].OnUpdate(deltaTime);
	}
}

void KatamariGame::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	for (string name : m_names)
	{
		if (m_objects.find(name) == m_objects.end()) { continue; }
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
			Add(commandList, name, "../../DX12GE/Resources/Katamari Objects/field/cube/cube.obj");
			m_objects[name].SetScale(cellSize, 0.01f, cellSize);
			float offset = fieldSize % 2 == 1 ? cellSize : 0;
			m_objects[name].SetPosition((i - fieldSize * 0.5f) * cellSize * 2 + cellSize, -0.01f / 2.0f , (j - fieldSize * 0.5f) * cellSize * 2 + cellSize);
		}
	}
}

void KatamariGame::Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path)
{
	// KEY found
	if (m_objects.find(name) != m_objects.end()) 
	{ 
		return; 
	}

	m_names.push_back(name);
	m_objects.insert({ name, BianObject() });
	m_objects[name].OnLoad(commandList, path);
}

void KatamariGame::Remove(string name)
{
	
}

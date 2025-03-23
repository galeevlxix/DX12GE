#include "KatamariGame.h"

int GetRandomNumber(int start, int end)
{
	return rand() % (end - start + 1) + start;
}

void KatamariGame::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	player.OnLoad(commandList);

	CreateField(commandList);		

	srand(time(0));

	for (int i = 0; i < itemCount; i++)
	{
		string name = "boot" + to_string(i);
		Add(commandList, name, "../../DX12GE/Resources/Katamari Objects/low-poly-boot/model.fbx");
		Vector3 randVector = Vector3(GetRandomNumber(-60, 60), GetRandomNumber(-100, 100) / 100.0 * PI, GetRandomNumber(-60, 60));
		m_objects[name].Move(randVector.x, 0, randVector.z);
		m_objects[name].SetRotationY(randVector.y);
		m_objects[name].SetScale(20, 20, 20);
		m_objects[name].canEatIt = true;
	}

	for (int i = 0; i < itemCount; i++)
	{
		string name = "cup" + to_string(i);
		Add(commandList, name, "../../DX12GE/Resources/Katamari Objects/low-poly-cup-with-lemon-tea/Cup.fbx");
		Vector3 randVector = Vector3(GetRandomNumber(-60, 60), GetRandomNumber(-100, 100) / 100.0 * PI, GetRandomNumber(-60, 60));
		m_objects[name].Move(randVector.x, 0, randVector.z);
		m_objects[name].SetRotationY(randVector.y);
		m_objects[name].SetScale(0.3f, 0.3f, 0.3f);
		m_objects[name].canEatIt = true;
	}

	for (int i = 0; i < itemCount; i++)
	{
		string name = "juice" + to_string(i);
		Add(commandList, name, "../../DX12GE/Resources/Katamari Objects/low-poly-stylized-juice/model.dae");
		Vector3 randVector = Vector3(GetRandomNumber(-60, 60), GetRandomNumber(-100, 100) / 100.0 * PI, GetRandomNumber(-60, 60));
		m_objects[name].Move(randVector.x, 2, randVector.z);
		m_objects[name].SetRotationY(randVector.y);
		m_objects[name].SetScale(150, 150, 150);
		m_objects[name].canEatIt = true;
	}

	for (int i = 0; i < itemCount; i++)
	{
		string name = "chair" + to_string(i);
		Add(commandList, name, "../../DX12GE/Resources/Katamari Objects/old-wooden-chair-low-poly/chair.fbx");
		Vector3 randVector = Vector3(GetRandomNumber(-60, 60), GetRandomNumber(-100, 100) / 100.0 * PI, GetRandomNumber(-60, 60));
		m_objects[name].Move(randVector.x, 0, randVector.z);
		m_objects[name].SetRotationY(randVector.y);
		m_objects[name].SetScale(0.05, 0.05, 0.05);
		m_objects[name].canEatIt = true;
	}

	for (int i = 0; i < itemCount; i++)
	{
		string name = "toothbrush" + to_string(i);
		Add(commandList, name, "../../DX12GE/Resources/Katamari Objects/toothbrush/model.dae");
		Vector3 randVector = Vector3(GetRandomNumber(-60, 60), GetRandomNumber(-100, 100) / 100.0 * PI, GetRandomNumber(-60, 60));
		m_objects[name].Move(randVector.x, -98, randVector.z);
		m_objects[name].SetScale(0.03, 0.03, 0.03);
		m_objects[name].canEatIt = true;
	}
}

void KatamariGame::CheckCollisions()
{
	static float step = 0.5;

	for (string name : m_names)
	{
		if (m_objects.find(name) == m_objects.end() || string::npos != name.find("field") || m_objects[name].eaten || !m_objects[name].canEatIt) { continue; }
		
		Vector3 objPos = m_objects[name].Position;

		float dx = abs(objPos.x - player.ball.Position.x);
		float dz = abs(objPos.z - player.ball.Position.z);

		if (dx * dx + dz * dz < player.ballRadius * player.ballRadius)
		{
			m_objects[name].eaten = true;

			player.ballRadius += step;
			float ratio = player.ballRadius / (player.ballRadius - step);

			player.ball.Expand(ratio);

			player.ball.SetPosition(player.ball.Position.x, player.ballRadius, player.ball.Position.z);
			//player.flyRadius *= ratio;
		}
	}
}

void KatamariGame::OnUpdate(float deltaTime)
{
	player.OnUpdate(deltaTime);

	CheckCollisions();

	for (string name : m_names)
	{
		if (m_objects.find(name) == m_objects.end() || m_objects[name].eaten) { continue; }
		m_objects[name].OnUpdate(deltaTime);
	}
}

void KatamariGame::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	player.OnRender(commandList, viewProjMatrix);
	for (string name : m_names)
	{
		if (m_objects.find(name) == m_objects.end() || m_objects[name].eaten) { continue; }
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

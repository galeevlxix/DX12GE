#pragma once

#include "ShaderResources.h"
#include "../../Game/KatamariGame.h"
#include <vector>

using namespace std;

class LightManager
{
private:
	Attenuation m_DefaultAttenuation = { 1.0f, 0.09f, 0.032f };
	
	float defaultIntensity = 0.75;
	float speed = PI / 4.0f;

	void AddPLights(Vector3 start, Vector3 end);

	Player* m_player;

public:
	void Init(KatamariGame* game);
	void OnUpdate(float deltaTime);

	vector<PointLight> m_PointLights;
	vector<SpotLight> m_SpotLights;
};
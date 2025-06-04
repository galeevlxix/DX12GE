#pragma once

#include "ShaderResources.h"
#include "../../Game/Player.h"
#include <vector>

using namespace std;

class LightManager
{
private:
	Attenuation m_DefaultAttenuation = { 1.0f, 0.09f, 0.032f };
	
	float step = PI / 7.0f;
	float max_radius = 50.0f;
	float radius = 50.0f;
	float defaultIntensity = 1.0f;
	float defaultHeight = 8.0f;

	float speed = PI / 4.0f;

	Player* m_player;

public:
	void Init(Player* player);
	void OnUpdate(float deltaTime);

	vector<PointLight> m_PointLights;
	vector<SpotLight> m_SpotLights;
};
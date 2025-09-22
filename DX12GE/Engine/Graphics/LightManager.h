#pragma once

#include <vector>
#include "ShaderResources.h"
#include "../../Game/ThirdPersonPlayer.h"

using namespace std;

class LightManager
{
private:
	Attenuation m_DefaultAttenuation = { 1.0f, 0.09f, 0.032f };
	
	float defaultIntensity = 0.75;
	float speed = PI / 4.0f;

	void AddPLights(Vector3 start, Vector3 end);

	ThirdPersonPlayer* m_player;

public:
	void Init(ThirdPersonPlayer* player);
	void OnUpdate(float deltaTime);

	vector<PointLight> m_PointLights;
	vector<SpotLight> m_SpotLights;
};
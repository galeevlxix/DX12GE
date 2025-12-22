#pragma once

#include <vector>
#include "ShaderResources.h"
#include "../Base/Singleton.h"

using namespace std;

class LightManager
{
	BaseLightComponent defaultBaseLight = BaseLightComponent();
	DirectionalLightComponent defaultDirLight = DirectionalLightComponent();

public:
	void OnUpdate(float deltaTime);

	void AddPointLight(Vector3 pos, Vector3 color, float intensity = 1.0f, float AttenConst = 1.0f, float AttenLinear = 0.09f, float AttenExp = 0.032f);
	void AddPointLight(Vector3 pos, Vector3 color, float intensity, AttenuationComponent atten);
	void AddSpotLight(Vector3 pos, Vector3 color, Vector3 dir, float cutoff = 0.65f, float intensity = 1.0f, float AttenConst = 1.0f, float AttenLinear = 0.09f, float AttenExp = 0.032f);
	void AddSpotLight(Vector3 pos, Vector3 color, Vector3 dir, float cutoff, float intensity, AttenuationComponent atten);
};
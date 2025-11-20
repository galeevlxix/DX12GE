#pragma once

#include <vector>
#include "ShaderResources.h"
#include "../../Game/ThirdPersonPlayer.h"
#include "DebugRenderSystem.h"

using namespace std;

class LightManager
{
public:
	vector<PointLight> m_PointLights;
	vector<SpotLight> m_SpotLights;

private:
	Attenuation m_DefaultAttenuation = { 1.0f, 0.09f, 0.032f };
	float defaultIntensity = 0.75;
	ThirdPersonPlayer* m_player;

public:
	void Init(ThirdPersonPlayer* player);
	void OnUpdate(float deltaTime);
	void DrawDebug(std::shared_ptr<DebugRenderSystem> debugRender);

	void AddPointLight(Vector3 pos, Vector3 color, float intensity = 1.0f, float AttenConst = 1.0f, float AttenLinear = 0.09f, float AttenExp = 0.032f);
	void AddPointLight(Vector3 pos, Vector3 color, float intensity, Attenuation atten);
	void AddSpotLight(Vector3 pos, Vector3 color, Vector3 dir, float cutoff = 0.65f, float intensity = 1.0f, float AttenConst = 1.0f, float AttenLinear = 0.09f, float AttenExp = 0.032f);
	void AddSpotLight(Vector3 pos, Vector3 color, Vector3 dir, float cutoff, float intensity, Attenuation atten);

private:
	void AddPLights(Vector3 start, Vector3 end);
};
#pragma once

#include "Player.h"
#include <vector>

using namespace std;

class LightManager
{
private:
	struct BaseLight
	{
		Vector3 Color;
		float Intensity;
	};

	struct DirectionalLight
	{
		BaseLight BaseLightComponent;
		Vector3 Direction;
	};

	struct Attenuation
	{
		float Constant;
		float Linear;
		float Exp;
	};

	Attenuation m_DefaultAttenuation = { 1.0f, 0.09f, 0.032f };

	struct LightProperties
	{
		UINT PointLightsCount;
		UINT SpotlightsCount;
	};

	struct PointLight
	{
		BaseLight BaseLightComponent;
		Vector3 Position;
		Attenuation AttenuationComponent;
	};

	struct SpotLight
	{
		PointLight PointLightComponent;
		Vector3 Direction;
		float Cutoff;
	};	

	struct SpecularLight
	{
		Vector3 CameraPos;
		float SpecularIntensity;
		float MaterialPower;
	};

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

	BaseLight m_AmbientLight;
	DirectionalLight m_DirectionalLight;
	LightProperties m_LightProperties;
	vector<PointLight> m_PointLights;
	vector<SpotLight> m_SpotLights;
	SpecularLight m_SpecularProperties;

	vector<BianObject> m_lamps;

	static UINT SizeOfAmbientLight()
	{
		return sizeof(BaseLight);
	}

	static UINT SizeOfDirectionalLight()
	{
		return sizeof(DirectionalLight);
	}

	static UINT SizeOfPointLight()
	{
		return sizeof(PointLight);
	}

	static UINT SizeOfSpotLight()
	{
		return sizeof(SpotLight);
	}

	static UINT SizeOfLightProperties()
	{
		return sizeof(LightProperties);
	}

	static UINT SizeOfSpecularLight()
	{
		return sizeof(SpecularLight);
	}
};
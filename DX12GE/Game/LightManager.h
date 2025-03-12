#include "../Engine/Vector3.h"
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

	Attenuation m_DefaultAttenuation = { 1, 0.09, 0.032 };

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
		Attenuation AttenuationComponent;
		float Cutoff;
	};	

public:

	LightManager();

	BaseLight m_AmbientLight;
	DirectionalLight m_DirectionalLight;
	LightProperties m_LightProperties;
	vector<PointLight> m_PointLights;
	vector<SpotLight> m_SpotLights;

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
};
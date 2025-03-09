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
		Attenuation AttenuationComponent;
	};

	

public:

	LightManager();

	BaseLight m_AmbientLight;
	DirectionalLight m_DirectionalLight;
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

};
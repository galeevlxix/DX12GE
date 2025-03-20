#include "LightManager.h"

#define COLOR_WHITE Vector3(1, 1, 1)
#define COLOR_BLACK Vector3(0, 0, 0)
#define COLOR_GRAY Vector3(0.5, 0.5, 0.5)

#define COLOR_RED Vector3(1, 0, 0)
#define COLOR_ORANGE Vector3(1, 0.5, 0)
#define COLOR_YELLOW Vector3(1, 1, 0)
#define COLOR_GREEN Vector3(0, 1, 0)
#define COLOR_CYAN Vector3(0, 0.5, 1)
#define COLOR_BLUE Vector3(0, 0, 1)
#define COLOR_PURPLE Vector3(0.5, 0, 1)

void LightManager::Init(Player* player)
{
	m_player = player;

	// AmbientLight
	m_AmbientLight.Color = COLOR_WHITE;
	m_AmbientLight.Intensity = 0.2;

	// DirectionalLight
	m_DirectionalLight.BaseLightComponent.Color = COLOR_WHITE;
	m_DirectionalLight.BaseLightComponent.Intensity = 0.2;
	m_DirectionalLight.Direction = Vector3(1, -1, 1);

	m_LightProperties.PointLightsCount = 14;
	m_LightProperties.SpotlightsCount = 2;

	m_SpotLights.push_back(SpotLight());
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Color = COLOR_WHITE;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Intensity = defaultIntensity;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.Position = Vector3(0, defaultHeight, 0);
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.AttenuationComponent = m_DefaultAttenuation;
	m_SpotLights[m_SpotLights.size() - 1].Direction = Vector3(0, -1, 0);
	m_SpotLights[m_SpotLights.size() - 1].Cutoff = 0.8f;

	m_SpotLights.push_back(SpotLight());
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Color = COLOR_WHITE;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Intensity = 8;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.Position = (*player).ball.Position;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.AttenuationComponent = m_DefaultAttenuation;
	m_SpotLights[m_SpotLights.size() - 1].Direction = (*player).Direction;
	m_SpotLights[m_SpotLights.size() - 1].Cutoff = 0.65f;

	for (int i = 0; i < 2; i++)
	{
		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_RED;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(cos(0 * step + PI * i) * radius, defaultHeight, sin(0 * step + PI * i) * radius);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_ORANGE;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(cos(1 * step + PI * i) * radius, defaultHeight, sin(1 * step + PI * i) * radius);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_YELLOW;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(cos(2 * step + PI * i) * radius, defaultHeight, sin(2 * step + PI * i) * radius);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_GREEN;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(cos(3 * step + PI * i) * radius, defaultHeight, sin(3 * step + PI * i) * radius);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_CYAN;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(cos(4 * step + PI * i) * radius, defaultHeight, sin(4 * step + PI * i) * radius);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_BLUE;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(cos(5 * step + PI * i) * radius, defaultHeight, sin(5 * step + PI * i) * radius);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_PURPLE;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(cos(6 * step + PI * i) * radius, defaultHeight, sin(6 * step + PI * i) * radius);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;
	}
}

void LightManager::OnUpdate(float deltaTime)
{
	static float path = 0;

	path += speed * 2 * deltaTime;
	if (path >= 2 * PI) path -= 2 * PI;

	radius = max_radius * ((sin(path) + 1.0) / 2.0 + 0.3);


	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			m_PointLights[j + 7 * i].Position = Vector3(cos(j * step + PI * i + path) * radius, defaultHeight, sin(j * step + PI * i + path) * radius);
		}
	}

	m_SpotLights[1].PointLightComponent.Position = (*m_player).ball.Position;
	m_SpotLights[1].Direction = (*m_player).Direction;
}

void LightManager::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	
}

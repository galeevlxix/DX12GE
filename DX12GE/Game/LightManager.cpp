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


int GetRandom(int start, int end)
{
	return rand() % (end - start + 1) + start;
}

LightManager::LightManager()
{
	// AmbientLight
	m_AmbientLight.Color = COLOR_WHITE;
	m_AmbientLight.Intensity = 0.2;

	// DirectionalLight
	m_DirectionalLight.BaseLightComponent.Color = COLOR_WHITE;
	m_DirectionalLight.BaseLightComponent.Intensity = 0.2;
	m_DirectionalLight.Direction = Vector3(1, -1, 1);

	m_LightProperties.PointLightsCount = 35;

	m_LightProperties.SpotlightsCount = 0;

	float defaultIntensity = 0.75;
	float defaultHeight = 6;

	float width = 5;

	for (int i = 0; i < width; i++)
	{
		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_RED;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-50 + i * 100.0 / width, defaultHeight, -50 + 0 * 15);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_ORANGE;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-50 + i * 100.0 / width, defaultHeight, -50 + 1 * 15);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_YELLOW;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-50 + i * 100.0 / width, defaultHeight, -50 + 2 * 15);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_GREEN;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-50 + i * 100.0 / width, defaultHeight, -50 + 3 * 15);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_CYAN;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-50 + i * 100.0 / width, defaultHeight, -50 + 4 * 15);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_BLUE;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-50 + i * 100.0 / width, defaultHeight, -50 + 5 * 15);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = COLOR_PURPLE;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-50 + i * 100.0 / width, defaultHeight, -50 + 6 * 15);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;
	}

	
}

void LightManager::CreateLamps(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	for (int i = 0; i < m_PointLights.size(); i++)
	{
		m_lamps.push_back(BianObject());
		m_lamps[i].OnLoad(commandList, "../../DX12GE/Resources/Katamari Objects/field/cube.obj");
		m_lamps[i].Move(0, 2, 0);
		m_lamps[i].SetRotation(0, 0, 0);
		m_lamps[i].SetScale(1, 1, 1);
	}
}

void LightManager::OnUpdate(float deltaTime)
{
	for (BianObject obj : m_lamps)
	{
		//obj.Move(0, -3 * deltaTime, 0);
		obj.OnUpdate(deltaTime);
	}
}

void LightManager::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	for (BianObject obj : m_lamps)
	{
		obj.OnRender(commandList, viewProjMatrix);
	}
}

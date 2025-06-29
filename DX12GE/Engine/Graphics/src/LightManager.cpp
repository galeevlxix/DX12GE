#include "../LightManager.h"

#define COLOR_WHITE		Vector3(1, 1, 1)
#define COLOR_BLACK		Vector3(0, 0, 0)
#define COLOR_GRAY		Vector3(0.5, 0.5, 0.5)

#define COLOR_RED		Vector3(1, 0, 0)
#define COLOR_ORANGE	Vector3(1, 0.5, 0)
#define COLOR_YELLOW	Vector3(1, 1, 0)
#define COLOR_GREEN		Vector3(0, 1, 0)
#define COLOR_CYAN		Vector3(0, 0.5, 1)
#define COLOR_BLUE		Vector3(0, 0, 1)
#define COLOR_PURPLE	Vector3(0.5, 0, 1)

void LightManager::Init(Player* player)
{
	m_player = player;

	// AmbientLight
	ShaderResources::GetWorldCB()->AmbientLight.Color = COLOR_WHITE;
	ShaderResources::GetWorldCB()->AmbientLight.Intensity = 0.1;

	// Specular
	ShaderResources::GetWorldCB()->LightProps.SpecularIntensity = 0.5f;
	ShaderResources::GetWorldCB()->LightProps.MaterialPower = 64;

	// DirectionalLight
	ShaderResources::GetWorldCB()->DirLight.BaseLightComponent.Color = COLOR_WHITE;
	ShaderResources::GetWorldCB()->DirLight.BaseLightComponent.Intensity = 0.5;
	ShaderResources::GetWorldCB()->DirLight.Direction = Vector4(1, -1, -1, 1);

	ShaderResources::GetWorldCB()->LightProps.PointLightsCount = 14;
	ShaderResources::GetWorldCB()->LightProps.SpotlightsCount = 2;

	m_SpotLights.push_back(SpotLight());
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Color = COLOR_WHITE;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Intensity = defaultIntensity * 6;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.Position = Vector3(0, 15, 0);
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.AttenuationComponent = m_DefaultAttenuation;
	m_SpotLights[m_SpotLights.size() - 1].Direction = Vector3(0, -1, 0.001);
	m_SpotLights[m_SpotLights.size() - 1].Cutoff = 0.7f;

	m_SpotLights.push_back(SpotLight());
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Color = COLOR_WHITE;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Intensity = defaultIntensity * 2;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.Position = (*player).prince.Transform.GetPosition();
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

	// ������������ ������������� ������� ��� ������� ���������
	for (size_t i = 0; i < m_PointLights.size(); i++)
	{
		auto pLight = m_PointLights[i];
		pLight.BaseLightComponent.Color.Normalize();
		auto col = max({ pLight.BaseLightComponent.Color.x, pLight.BaseLightComponent.Color.y, pLight.BaseLightComponent.Color.z });
		float a = pLight.AttenuationComponent.Exp;
		float b = pLight.AttenuationComponent.Linear;
		float c = pLight.AttenuationComponent.Constant - col * pLight.BaseLightComponent.Intensity * 128;
		float desc = b * b - 4 * a * c;
		float rad = (-b + sqrtf(desc)) / (2 * a);
		m_PointLights[i].MaxRadius = rad;
	}

	for (size_t i = 0; i < m_SpotLights.size(); i++)
	{
		auto pLight = m_SpotLights[i];
		pLight.PointLightComponent.BaseLightComponent.Color.Normalize();
		auto col = max({ pLight.PointLightComponent.BaseLightComponent.Color.x, pLight.PointLightComponent.BaseLightComponent.Color.y, pLight.PointLightComponent.BaseLightComponent.Color.z });
		float a = pLight.PointLightComponent.AttenuationComponent.Exp;
		float b = pLight.PointLightComponent.AttenuationComponent.Linear;
		float c = pLight.PointLightComponent.AttenuationComponent.Constant - col * pLight.PointLightComponent.BaseLightComponent.Intensity * 128;
		float desc = b * b - 4 * a * c;
		float rad = (-b + sqrtf(desc)) / (2 * a);
		m_SpotLights[i].PointLightComponent.MaxRadius = rad;
	}
}

void LightManager::OnUpdate(float deltaTime)
{
	static float path = 0;

	path += speed * deltaTime;
	if (path >= 2 * PI) path -= 2 * PI;

	radius = max_radius * ((sin(path) + 1.0) / 2.0 + 0.3);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			m_PointLights[j + 7 * i].Position = Vector3(cos(j * step + PI * i + path) * radius, defaultHeight, sin(j * step + PI * i + path) * radius);
		}
	}

	ShaderResources::GetWorldCB()->DirLight.Direction.x = sin(path);
	ShaderResources::GetWorldCB()->DirLight.Direction.z = cos(path);

	m_SpotLights[1].PointLightComponent.Position = (*m_player).prince.Transform.GetPosition() + Vector3(0, 2, 0);
	m_SpotLights[1].Direction = (*m_player).Direction;
	m_SpotLights[1].PointLightComponent.BaseLightComponent.Intensity == 10;
}

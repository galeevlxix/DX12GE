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

#define PURPLE_LIGHT	Vector3(0.54902, 0.0, 0.8549)
#define WHITE_LIGHT		Vector3(1.0, 0.87059, 0.71373)

void LightManager::Init(KatamariGame* game)
{
	m_player = &game->player;

	// AmbientLight
	ShaderResources::GetWorldCB()->AmbientLight.Color = COLOR_WHITE;
	ShaderResources::GetWorldCB()->AmbientLight.Intensity = 0.2;

	// Specular
	ShaderResources::GetWorldCB()->LightProps.SpecularIntensity = 1.0f;
	ShaderResources::GetWorldCB()->LightProps.MaterialPower = 32.0;

	// DirectionalLight
	ShaderResources::GetWorldCB()->DirLight.BaseLightComponent.Color = COLOR_WHITE;
	ShaderResources::GetWorldCB()->DirLight.BaseLightComponent.Intensity = 0.5;
	ShaderResources::GetWorldCB()->DirLight.Direction = Vector4(1, -1, -1, 1);

	m_SpotLights.push_back(SpotLight());
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Color = COLOR_WHITE;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Intensity = defaultIntensity * 2;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.Position = (*m_player).prince.Transform.GetPosition();
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.AttenuationComponent = m_DefaultAttenuation;
	m_SpotLights[m_SpotLights.size() - 1].Direction = (*m_player).Direction;
	m_SpotLights[m_SpotLights.size() - 1].Cutoff = 0.65f;

	
	{
		// white

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(5.7, 13.7, 3.0);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(5.7, 8.9, 3.0);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(20.1, 19.6, 11.2);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(16.1, 19.6, 11.2);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		{
			Vector3 start = Vector3(5.7, 10.6, -25.9);
			Vector3 end = Vector3(5.7, 15.0, -7.9);

			float stepZ = (end.z - start.z) / (6.0 - 1.0);
			float stepY = (end.y - start.y) / (3.0 - 1.0);

			for (size_t i = 0; i < 6; i++)
			{
				for (size_t j = 0; j < 3; j++)
				{
					m_PointLights.push_back(PointLight());
					m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
					m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
					m_PointLights[m_PointLights.size() - 1].Position = start + Vector3(0, j * stepY, i * stepZ);
					m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;
				}
			}
		}		

		// purple

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = PURPLE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(7.4, 18.0, 12.7);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = PURPLE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(7.4, 16.2, 12.7);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = PURPLE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-26.2, 18.7, 10.2);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = PURPLE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-26.2, 18.7, 8.3);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		AddPLights(Vector3(5.8, 18.8, -26.9), Vector3(-17.4, 18.8, -13.2));
		AddPLights(Vector3(-17.4, 18.8, -13.2), Vector3(-17.4, 18.8, 5.9));
		AddPLights(Vector3(-17.4, 18.8, 5.9), Vector3(5.8, 18.8, 5.9));

		AddPLights(Vector3(-14.7, 17.8, -0.2), Vector3(-14.7, 6.3, -3.7));
		AddPLights(Vector3(-14.7, 17.8, -6.1), Vector3(-14.7, 6.3, -6.1));
		AddPLights(Vector3(-5.7, 17.0, -2.1), Vector3(-5.7, 17.0, -12.2));
	}
  

	// кол-ва источников света
	ShaderResources::GetWorldCB()->LightProps.PointLightsCount = m_PointLights.size();
	ShaderResources::GetWorldCB()->LightProps.SpotlightsCount = m_SpotLights.size();

	// высчитывание максимального радиуса для каждого источника
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

void LightManager::AddPLights(Vector3 start, Vector3 end)
{
	float step_length = 3.7f;

	Vector3 vec = end - start;
	float dist = vec.Length();

	Vector3 step;
	vec.Normalize(step);
	step *= step_length;

	int step_count = dist / step_length;
	Vector3 cur_pos = start;

	for (size_t i = 0; i <= step_count; i++)
	{
		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = PURPLE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = cur_pos;
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		cur_pos += step;
	}
}

void LightManager::OnUpdate(float deltaTime)
{
	static float path = 0;

	path += speed * deltaTime;
	if (path >= 2 * PI) path -= 2 * PI;

	ShaderResources::GetWorldCB()->DirLight.Direction.x = sin(path);
	ShaderResources::GetWorldCB()->DirLight.Direction.z = cos(path);

	m_SpotLights[0].PointLightComponent.Position = (*m_player).prince.Transform.GetPosition() + Vector3(0, 2, 0);
	m_SpotLights[0].Direction = (*m_player).Direction;
	m_SpotLights[0].PointLightComponent.BaseLightComponent.Intensity == 10;
}

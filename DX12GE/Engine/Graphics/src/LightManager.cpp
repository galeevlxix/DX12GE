#include "../LightManager.h"

#define COLOR_WHITE		Vector3(1.0f, 1.0f, 1.0f)
#define COLOR_BLACK		Vector3(0.0f, 0.0f, 0.0f)
#define COLOR_GRAY		Vector3(0.5f, 0.5f, 0.5f)

#define COLOR_RED		Vector3(1.0f, 0.0f, 0.0f)
#define COLOR_ORANGE	Vector3(1.0f, 0.5f, 0.0f)
#define COLOR_YELLOW	Vector3(1.0f, 1.0f, 0.0f)
#define COLOR_GREEN		Vector3(0.0f, 1.0f, 0.0f)
#define COLOR_CYAN		Vector3(0.0f, 0.5f, 1.0f)
#define COLOR_BLUE		Vector3(0.0f, 0.0f, 1.0f)
#define COLOR_PURPLE	Vector3(0.5f, 0.0f,	1.0f)

#define PURPLE_LIGHT	Vector3(0.54902f,	0.0f,		0.8549f)
#define WHITE_LIGHT		Vector3(1.0f,		0.87059f,	0.71373f)

void LightManager::Init(KatamariGame* game)
{
	m_player = &game->player;

	// AmbientLight
	ShaderResources::GetWorldCB()->AmbientLight.Color = COLOR_WHITE;
	ShaderResources::GetWorldCB()->AmbientLight.Intensity = 0.2f;

	// Specular
	ShaderResources::GetWorldCB()->LightProps.SpecularIntensity = 1.0f;
	ShaderResources::GetWorldCB()->LightProps.MaterialPower = 32.0f;

	// DirectionalLight
	ShaderResources::GetWorldCB()->DirLight.BaseLightComponent.Color = COLOR_WHITE;
	ShaderResources::GetWorldCB()->DirLight.BaseLightComponent.Intensity = 0.5f;
	ShaderResources::GetWorldCB()->DirLight.Direction = Vector4(1.0f, -1.0f, -1.0f, 1.0f);

	m_SpotLights.push_back(SpotLight());
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Color = COLOR_WHITE;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.BaseLightComponent.Intensity = defaultIntensity * 2.0f;
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.Position = (*m_player).prince.Transform.GetPosition();
	m_SpotLights[m_SpotLights.size() - 1].PointLightComponent.AttenuationComponent = m_DefaultAttenuation;
	m_SpotLights[m_SpotLights.size() - 1].Direction = (*m_player).Direction;
	m_SpotLights[m_SpotLights.size() - 1].Cutoff = 0.65f;

	
	{
		// white

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(5.7f, 13.7f, 3.0f);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(5.7f, 8.9f, 3.0f);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(20.1f, 19.6f, 11.2f);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = WHITE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(16.1f, 19.6f, 11.2f);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		{
			Vector3 start = Vector3(5.7f, 10.6f, -25.9f);
			Vector3 end = Vector3(5.7f, 15.0f, -7.9f);

			float stepZ = (end.z - start.z) / (6.0f - 1.0f);
			float stepY = (end.y - start.y) / (3.0f - 1.0f);

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
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(7.4f, 18.0f, 12.7f);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = PURPLE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(7.4f, 16.2f, 12.7f);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = PURPLE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-26.2f, 18.7f, 10.2f);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		m_PointLights.push_back(PointLight());
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Color = PURPLE_LIGHT;
		m_PointLights[m_PointLights.size() - 1].BaseLightComponent.Intensity = defaultIntensity;
		m_PointLights[m_PointLights.size() - 1].Position = Vector3(-26.2f, 18.7f, 8.3f);
		m_PointLights[m_PointLights.size() - 1].AttenuationComponent = m_DefaultAttenuation;

		AddPLights(Vector3(5.8f,	18.8f,	-26.9f),	Vector3(-17.4f, 18.8f, -13.2f));
		AddPLights(Vector3(-17.4f,	18.8f,	-13.2f),	Vector3(-17.4f, 18.8f, 5.9f));
		AddPLights(Vector3(-17.4f,	18.8f,	5.9f),		Vector3(5.8f,	18.8f, 5.9f));

		AddPLights(Vector3(-14.7f,	17.8f,	-0.2f),		Vector3(-14.7f, 6.3f,	-3.7f));
		AddPLights(Vector3(-14.7f,	17.8f,	-6.1f),		Vector3(-14.7f, 6.3f,	-6.1f));
		AddPLights(Vector3(-5.7f,	17.0f,	-2.1f),		Vector3(-5.7f,	17.0f,	-12.2f));
	}

	// кол-ва источников света
	ShaderResources::GetWorldCB()->LightProps.PointLightsCount = static_cast<float>(m_PointLights.size());
	ShaderResources::GetWorldCB()->LightProps.SpotlightsCount = static_cast<float>(m_SpotLights.size());

	// высчитывание максимального радиуса для каждого источника
	for (size_t i = 0; i < m_PointLights.size(); i++)
	{
		auto pLight = m_PointLights[i];
		pLight.BaseLightComponent.Color.Normalize();
		auto col = max({ pLight.BaseLightComponent.Color.x, pLight.BaseLightComponent.Color.y, pLight.BaseLightComponent.Color.z });
		float a = pLight.AttenuationComponent.Exp;
		float b = pLight.AttenuationComponent.Linear;
		float c = pLight.AttenuationComponent.Constant - col * pLight.BaseLightComponent.Intensity * 128.0f;
		float desc = b * b - 4.0f * a * c;
		float rad = (-b + sqrtf(desc)) / (2.0f * a);
		m_PointLights[i].MaxRadius = rad;
	}

	for (size_t i = 0; i < m_SpotLights.size(); i++)
	{
		auto pLight = m_SpotLights[i];
		pLight.PointLightComponent.BaseLightComponent.Color.Normalize();
		auto col = max({ pLight.PointLightComponent.BaseLightComponent.Color.x, pLight.PointLightComponent.BaseLightComponent.Color.y, pLight.PointLightComponent.BaseLightComponent.Color.z });
		float a = pLight.PointLightComponent.AttenuationComponent.Exp;
		float b = pLight.PointLightComponent.AttenuationComponent.Linear;
		float c = pLight.PointLightComponent.AttenuationComponent.Constant - col * pLight.PointLightComponent.BaseLightComponent.Intensity * 128.0f;
		float desc = b * b - 4.0f * a * c;
		float rad = (-b + sqrtf(desc)) / (2.0f * a);
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

	int step_count = static_cast<int>(dist / step_length);
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
	static float path = 0.0f;

	path += speed * deltaTime;
	if (path >= 2.0f * PI) path -= 2.0f * PI;

	ShaderResources::GetWorldCB()->DirLight.Direction.x = sin(path);
	ShaderResources::GetWorldCB()->DirLight.Direction.z = cos(path);

	m_SpotLights[0].PointLightComponent.Position = (*m_player).prince.Transform.GetPosition() + Vector3(0.0f, 2.0f, 0.0f);
	m_SpotLights[0].Direction = (*m_player).Direction;
	m_SpotLights[0].PointLightComponent.BaseLightComponent.Intensity = 5.0f;
}

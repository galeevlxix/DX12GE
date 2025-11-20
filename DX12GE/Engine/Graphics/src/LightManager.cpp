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

void LightManager::Init(ThirdPersonPlayer* player)
{
	m_player = player;

	// AmbientLight
	ShaderResources::GetWorldCB()->AmbientLight.Color = COLOR_WHITE;
	ShaderResources::GetWorldCB()->AmbientLight.Intensity = 0.2f;

	// Specular
	ShaderResources::GetWorldCB()->LightProps.SpecularIntensity = 1.0f;
	ShaderResources::GetWorldCB()->LightProps.MaterialPower = 64.0f;

	// DirectionalLight
	ShaderResources::GetWorldCB()->DirLight.BaseLightComponent.Color = COLOR_WHITE;
	ShaderResources::GetWorldCB()->DirLight.BaseLightComponent.Intensity = 1.0f;
	ShaderResources::GetWorldCB()->DirLight.Direction = Vector4(1.0f, -1.0f, -1.0f, 1.0f);

	AddSpotLight(m_player->Transform.GetPosition(), COLOR_WHITE, m_player->m_Direction, 0.65f, 5.0f);

	AddPointLight(Vector3(5.7f, 13.7f, 3.0f),	WHITE_LIGHT, defaultIntensity, m_DefaultAttenuation);
	AddPointLight(Vector3(5.7f, 8.9f, 3.0f),	WHITE_LIGHT, defaultIntensity, m_DefaultAttenuation);
	AddPointLight(Vector3(20.1f, 19.6f, 11.2f), WHITE_LIGHT, defaultIntensity, m_DefaultAttenuation);
	AddPointLight(Vector3(16.1f, 19.6f, 11.2f), WHITE_LIGHT, defaultIntensity, m_DefaultAttenuation);

	Vector3 start = Vector3(5.7f, 10.6f, -25.9f);
	Vector3 end = Vector3(5.7f, 15.0f, -7.9f);

	float stepZ = (end.z - start.z) / (6.0f - 1.0f);
	float stepY = (end.y - start.y) / (3.0f - 1.0f);

	for (size_t i = 0; i < 6; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			AddPointLight(start + Vector3(0, j * stepY, i * stepZ), WHITE_LIGHT, defaultIntensity, m_DefaultAttenuation);
		}
	}

	AddPointLight(Vector3(7.4f, 18.0f, 12.7f),	 PURPLE_LIGHT, defaultIntensity, m_DefaultAttenuation);
	AddPointLight(Vector3(7.4f, 16.2f, 12.7f),	 PURPLE_LIGHT, defaultIntensity, m_DefaultAttenuation);
	AddPointLight(Vector3(-26.2f, 18.7f, 10.2f), PURPLE_LIGHT, defaultIntensity, m_DefaultAttenuation);
	AddPointLight(Vector3(-26.2f, 18.7f, 8.3f),  PURPLE_LIGHT, defaultIntensity, m_DefaultAttenuation);

	AddPLights(Vector3(5.8f, 18.8f, -26.9f),	Vector3(-17.4f, 18.8f, -13.2f));
	AddPLights(Vector3(-17.4f, 18.8f, -13.2f),	Vector3(-17.4f, 18.8f, 5.9f));
	AddPLights(Vector3(-17.4f, 18.8f, 5.9f),	Vector3(5.8f, 18.8f, 5.9f));

	AddPLights(Vector3(-14.7f, 17.8f, -0.2f),	Vector3(-14.7f, 6.3f, -3.7f));
	AddPLights(Vector3(-14.7f, 17.8f, -6.1f),	Vector3(-14.7f, 6.3f, -6.1f));
	AddPLights(Vector3(-5.7f, 17.0f, -2.1f),	Vector3(-5.7f, 17.0f, -12.2f));
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
		AddPointLight(cur_pos, PURPLE_LIGHT, defaultIntensity, m_DefaultAttenuation);
		cur_pos += step;
	}
}

void LightManager::OnUpdate(float deltaTime)
{
	static float path = 0.0f;
	path += PI / 4.0f * deltaTime;
	if (path >= 2.0f * PI) path -= 2.0f * PI;

	ShaderResources::GetWorldCB()->DirLight.Direction.x = sin(path);
	ShaderResources::GetWorldCB()->DirLight.Direction.z = cos(path);

	m_SpotLights[0].PointLightComponent.Position = m_player->Transform.GetPosition() + Vector3(0.0f, 2.0f, 0.0f);
	m_SpotLights[0].Direction = m_player->m_Direction;

	// кол-ва источников света
	ShaderResources::GetWorldCB()->LightProps.PointLightsCount = static_cast<float>(m_PointLights.size());
	ShaderResources::GetWorldCB()->LightProps.SpotlightsCount = static_cast<float>(m_SpotLights.size());
}

void LightManager::AddPointLight(Vector3 pos, Vector3 color, float intensity, float AttenConst, float AttenLinear, float AttenExp)
{
	PointLight pLight;
	pLight.BaseLightComponent.Color = color;
	pLight.BaseLightComponent.Intensity = intensity;
	pLight.Position = pos;
	pLight.AttenuationComponent = { AttenConst, AttenLinear, AttenExp };
	m_PointLights.push_back(pLight);
}

void LightManager::AddPointLight(Vector3 pos, Vector3 color, float intensity, Attenuation atten)
{
	AddPointLight(pos, color, intensity, atten.Constant, atten.Linear, atten.Exp);
}

void LightManager::AddSpotLight(Vector3 pos, Vector3 color, Vector3 dir, float cutoff, float intensity, float AttenConst, float AttenLinear, float AttenExp)
{
	SpotLight sLight;
	sLight.PointLightComponent.BaseLightComponent.Color = color;
	sLight.PointLightComponent.BaseLightComponent.Intensity = intensity;
	sLight.PointLightComponent.Position = pos;
	sLight.PointLightComponent.AttenuationComponent = { AttenConst, AttenLinear, AttenExp };
	sLight.Direction = dir;
	sLight.Cutoff = cutoff;
	m_SpotLights.push_back(sLight);
}

void LightManager::AddSpotLight(Vector3 pos, Vector3 color, Vector3 dir, float cutoff, float intensity, Attenuation atten)
{
	AddSpotLight(pos, color, dir, cutoff, intensity, atten.Constant, atten.Linear, atten.Exp);
}

void LightManager::DrawDebug(std::shared_ptr<DebugRenderSystem> debugRender)
{
	for (auto pLight : m_PointLights)
	{
		debugRender->DrawPoint(pLight.Position, 1.0f, pLight.BaseLightComponent.Color);
	}

	for (auto sLight : m_SpotLights)
	{
		debugRender->DrawPoint(sLight.PointLightComponent.Position, 1.0f, sLight.PointLightComponent.BaseLightComponent.Color);
		debugRender->DrawArrow(
			sLight.PointLightComponent.Position,
			sLight.PointLightComponent.Position + sLight.Direction * 2.0f,
			sLight.PointLightComponent.BaseLightComponent.Color,
			Vector3(0, 1, 0));
	}
}
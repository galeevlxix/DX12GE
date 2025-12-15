#include "../LightManager.h"

void LightManager::OnUpdate(float deltaTime)
{
	// AmbientLight
	if (EnvironmentNode* env = Singleton::GetNodeGraph()->GetActiveEnvironment())
	{
		ShaderResources::GetWorldCB()->AmbientLight.Color = env->AmbientLightColor;
		ShaderResources::GetWorldCB()->AmbientLight.Intensity = env->AmbientLightIntensity;

		ShaderResources::GetWorldCB()->LightProps.FogColor = Vector4(env->FogColor);
		ShaderResources::GetWorldCB()->LightProps.FogColor.w = env->FogEnabled ? 1.0f : 0.0f;
		ShaderResources::GetWorldCB()->LightProps.FogStart = env->FogStart;
		ShaderResources::GetWorldCB()->LightProps.FogDistance = env->FogDistance;

		ShaderResources::GetSSRCB()->MaxDistance = env->SSRMaxDistance;
		ShaderResources::GetSSRCB()->RayStep = env->SSRStepLength;
		ShaderResources::GetSSRCB()->Thickness = env->SSRThickness;
	}	
	else
	{
		ShaderResources::GetWorldCB()->AmbientLight.Color = defaultBaseLight.Color;
		ShaderResources::GetWorldCB()->AmbientLight.Intensity = defaultBaseLight.Intensity;

		ShaderResources::GetWorldCB()->LightProps.FogColor.x = 0.5f;
		ShaderResources::GetWorldCB()->LightProps.FogColor.y = 0.5f;
		ShaderResources::GetWorldCB()->LightProps.FogColor.z = 0.5f;
		ShaderResources::GetWorldCB()->LightProps.FogColor.w = 1.0f;
		ShaderResources::GetWorldCB()->LightProps.FogStart = 35.0f;
		ShaderResources::GetWorldCB()->LightProps.FogDistance = 230.0f;

		ShaderResources::GetSSRCB()->MaxDistance = 0.0f;
		ShaderResources::GetSSRCB()->RayStep = 1.0f;
		ShaderResources::GetSSRCB()->Thickness = 0.9f;
	}

	// DirectionalLight
	if (DirectionalLightNode* dirL = Singleton::GetNodeGraph()->GetActiveDirectionalLight())
	{
		ShaderResources::GetWorldCB()->DirLight.BaseLightProperties.Color = dirL->Color;
		ShaderResources::GetWorldCB()->DirLight.BaseLightProperties.Intensity = dirL->Intensity;
		ShaderResources::GetWorldCB()->DirLight.Direction = Vector4(dirL->GetWorldDirection());
	}
	else
	{
		ShaderResources::GetWorldCB()->DirLight.BaseLightProperties.Color = defaultDirLight.BaseLightProperties.Color;
		ShaderResources::GetWorldCB()->DirLight.BaseLightProperties.Intensity = defaultDirLight.BaseLightProperties.Intensity;
		ShaderResources::GetWorldCB()->DirLight.Direction = defaultDirLight.Direction;
	}	

	// кол-ва источников света
	ShaderResources::GetWorldCB()->LightProps.PointLightsCount = static_cast<float>(Singleton::GetNodeGraph()->GetPointLightsCount());
	ShaderResources::GetWorldCB()->LightProps.SpotlightsCount = static_cast<float>(Singleton::GetNodeGraph()->GetSpotLightsCount());
}

void LightManager::AddPointLight(Vector3 pos, Vector3 color, float intensity, float AttenConst, float AttenLinear, float AttenExp)
{
	PointLightComponent pLight;
	pLight.BaseLightProperties.Color = color;
	pLight.BaseLightProperties.Intensity = intensity;
	pLight.AttenuationProperties = { AttenConst, AttenLinear, AttenExp };

	if (Node3D* gas = Singleton::GetNodeGraph()->GetRoot()->GetChild("gas1"))
	{
		PointLightNode* pLightNode = new PointLightNode();
		pLightNode->LightData = pLight;
		pLightNode->Transform.SetPosition(pos);
		if (!gas->AddChild(pLightNode))
		{
			throw;
		}
	}
}

void LightManager::AddPointLight(Vector3 pos, Vector3 color, float intensity, AttenuationComponent atten)
{
	AddPointLight(pos, color, intensity, atten.Constant, atten.Linear, atten.Exp);
}

void LightManager::AddSpotLight(Vector3 pos, Vector3 color, Vector3 dir, float cutoff, float intensity, float AttenConst, float AttenLinear, float AttenExp)
{
	SpotLightComponent sLight;
	sLight.PointLightProperties.BaseLightProperties.Color = color;
	sLight.PointLightProperties.BaseLightProperties.Intensity = intensity;
	sLight.PointLightProperties.AttenuationProperties = { AttenConst, AttenLinear, AttenExp };
	sLight.Cutoff = cutoff;

	if (Node3D* gas = Singleton::GetNodeGraph()->GetRoot()->GetChild("gas2"))
	{
		Node3D* gas1 = Singleton::GetNodeGraph()->GetRoot()->GetChild("gas1");

		SpotLightNode* sLightNode = new SpotLightNode();
		Matrix m = gas->Transform.GetLocalMatrix();		
		sLightNode->Transform.SetPosition(Vector3::Transform(pos, m.Invert()));
		sLightNode->Transform.RotateDegrees(Vector3(90, 0, 0));
		sLightNode->LightData = sLight;
		if (!gas->AddChild(sLightNode))
		{
			throw;
		}
	}
}

void LightManager::AddSpotLight(Vector3 pos, Vector3 color, Vector3 dir, float cutoff, float intensity, AttenuationComponent atten)
{
	AddSpotLight(pos, color, dir, cutoff, intensity, atten.Constant, atten.Linear, atten.Exp);
}

void LightManager::DrawDebug()
{
	for (auto pLight : Singleton::GetNodeGraph()->GetActivePointLightComponents())
	{
		Singleton::GetDebugRender()->DrawPoint(pLight.WorldPosition, 0.5f, pLight.BaseLightProperties.Color);
	}

	for (auto sLight : Singleton::GetNodeGraph()->GetActiveSpotLightComponents())
	{
		Vector3 n = abs(sLight.Direction.y) == 1.0f ? Vector3(1, 0, 0) : Vector3(0, 1, 0);
		Singleton::GetDebugRender()->DrawArrow(
			sLight.PointLightProperties.WorldPosition,
			sLight.PointLightProperties.WorldPosition + sLight.Direction * 1.0f,
			sLight.PointLightProperties.BaseLightProperties.Color,
			n);
	}
}
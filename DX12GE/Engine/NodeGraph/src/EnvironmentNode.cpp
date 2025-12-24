#include "../../Base/Singleton.h"

EnvironmentNode::EnvironmentNode() : Node3D()
{
	m_Type = NODE_TYPE_ENVIRONMENT;
	AmbientLightData = BaseLightComponent();

	FogEnabled = true;
	FogColor = DirectX::SimpleMath::Vector3(0.5f, 0.5f, 0.5f);
	FogStart = 35.0f;
	FogDistance = 230.0f;

	SSRMaxDistance = 32.0f;
	SSRStepLength = 0.1f;
	SSRThickness = 0.0999f;
	
	Rename("EnvironmentNode");
}

void EnvironmentNode::Clone(Node3D* cloneNode, Node3D* newParrent, bool cloneChildrenRecursive)
{
	if (!cloneNode)
	{
		cloneNode = new EnvironmentNode();
	}

	Node3D::Clone(cloneNode, newParrent, cloneChildrenRecursive);

	if (cloneNode)
	{
		EnvironmentNode* env = dynamic_cast<EnvironmentNode*>(cloneNode);

		env->AmbientLightData = AmbientLightData;

		env->FogEnabled = FogEnabled;
		env->FogColor = FogColor;
		env->FogStart = FogStart;
		env->FogDistance = FogDistance;

		env->SSRMaxDistance = SSRMaxDistance;
		env->SSRStepLength = SSRStepLength;
		env->SSRThickness = SSRThickness;
	}
}

void EnvironmentNode::DrawDebug()
{
	Node3D::DrawDebug();
	Singleton::GetDebugRender()->DrawPoint(m_WorldPositionCache, 0.5f);
}

void EnvironmentNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);

	j["light_color_r"] = AmbientLightData.Color.x;
	j["light_color_g"] = AmbientLightData.Color.y;
	j["light_color_b"] = AmbientLightData.Color.z;
	j["light_intensity"] = AmbientLightData.Intensity;
	
	j["fog_enabled"] = FogEnabled;
	j["fog_color_r"] = FogColor.x;
	j["fog_color_g"] = FogColor.y;
	j["fog_color_b"] = FogColor.z;
	j["fog_start"] = FogStart;
	j["fog_distance"] = FogDistance;
	
	j["ssr_max_distance"] = SSRMaxDistance;
	j["ssr_step_length"] = SSRStepLength;
	j["ssr_thickness"] = SSRThickness;

	if (IsCurrent())
	{
		j["is_current"] = true;
	}
}

void EnvironmentNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	AmbientLightData.Color = nodeData.lightColor;
	AmbientLightData.Intensity = nodeData.lightIntensity;
	FogColor = nodeData.envFogColor;
	FogEnabled = nodeData.envFogEnabled;
	FogStart = nodeData.envFogStart;
	FogDistance = nodeData.envFogDistance;
	SSRMaxDistance = nodeData.envSSRMaxDistance;
	SSRStepLength = nodeData.envSSRStepLength;
	SSRThickness = nodeData.envSSRThickness;
}

void EnvironmentNode::SetCurrent()
{
	if (IsInsideTree())
	{
		Singleton::GetNodeGraph()->m_CurrentEnvironment = this;
	}
}

bool EnvironmentNode::IsCurrent()
{
	return Singleton::GetNodeGraph()->m_CurrentEnvironment == this;
}
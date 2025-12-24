#include "../../Base/Singleton.h"

SpotLightNode::SpotLightNode() : Node3D()
{
	m_Type = NODE_TYPE_SPOT_LIGHT;
	LightData = SpotLightComponent();
	Rename("SpotLightNode");
}

void SpotLightNode::OnUpdate(const double& deltaTime)
{
	Node3D::OnUpdate(deltaTime);
	LightData.WorldDirection = m_WorldDirectionCache;
	LightData.PointLightProperties.WorldPosition = m_WorldPositionCache;
}

void SpotLightNode::Clone(Node3D* cloneNode, Node3D* newParrent, bool cloneChildrenRecursive)
{
	if (!cloneNode)
	{
		cloneNode = new SpotLightNode();
	}

	Node3D::Clone(cloneNode, newParrent, cloneChildrenRecursive);

	if (cloneNode)
	{
		SpotLightNode* sLight = dynamic_cast<SpotLightNode*>(cloneNode);
		sLight->LightData = LightData;
	}
}

void SpotLightNode::DrawDebug()
{
	Node3D::DrawDebug();
	Singleton::GetDebugRender()->DrawArrow(
		m_WorldPositionCache,
		m_WorldPositionCache + m_WorldDirectionCache,
		LightData.PointLightProperties.BaseLightProperties.Color,
		abs(m_WorldDirectionCache.y) == 1.0f ? Vector3::UnitX : Vector3::UnitY);
}

void SpotLightNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);
	
	j["light_color_r"] = LightData.PointLightProperties.BaseLightProperties.Color.x;
	j["light_color_g"] = LightData.PointLightProperties.BaseLightProperties.Color.y;
	j["light_color_b"] = LightData.PointLightProperties.BaseLightProperties.Color.z;
	j["light_intensity"] = LightData.PointLightProperties.BaseLightProperties.Intensity;

	j["light_atten_constant"] = LightData.PointLightProperties.AttenuationProperties.Constant;
	j["light_atten_linear"] = LightData.PointLightProperties.AttenuationProperties.Linear;
	j["light_atten_exp"] = LightData.PointLightProperties.AttenuationProperties.Exp;

	j["light_cutoff"] = LightData.Cutoff;
}

void SpotLightNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	LightData.PointLightProperties.BaseLightProperties.Color = nodeData.lightColor;
	LightData.PointLightProperties.BaseLightProperties.Intensity = nodeData.lightIntensity;
	LightData.PointLightProperties.AttenuationProperties.Constant = nodeData.lightAttenuation.x;
	LightData.PointLightProperties.AttenuationProperties.Linear = nodeData.lightAttenuation.y;
	LightData.PointLightProperties.AttenuationProperties.Exp = nodeData.lightAttenuation.z;
	LightData.Cutoff = nodeData.lightCutoff;
}

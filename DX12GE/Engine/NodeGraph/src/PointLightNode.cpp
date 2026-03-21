#include "../../Base/Singleton.h"

PointLightNode::PointLightNode() : Node3D()
{
	m_Type = NODE_TYPE_POINT_LIGHT;
	LightData = PointLightComponent();
	Rename("PointLightNode");
}

void PointLightNode::OnUpdate(const double& deltaTime)
{
	Node3D::OnUpdate(deltaTime);
	LightData.WorldPosition = m_WorldPositionCache;
}

Node3D* PointLightNode::Clone(Node3D* newParrent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
	if (!cloneNode)
	{
		cloneNode = new PointLightNode();
	}

	Node3D::Clone(newParrent, cloneChildrenRecursive, cloneNode);

	if (cloneNode)
	{
		PointLightNode* pLight = dynamic_cast<PointLightNode*>(cloneNode);
		pLight->LightData = LightData;
	}

	return cloneNode;
}

void PointLightNode::DrawDebug()
{
	Node3D::DrawDebug();
	Singleton::GetDebugRender()->DrawPoint(m_WorldPositionCache, 0.5f, LightData.BaseLightProperties.Color);
}

void PointLightNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);
	
	j["light_color_r"] = LightData.BaseLightProperties.Color.x;
	j["light_color_g"] = LightData.BaseLightProperties.Color.y;
	j["light_color_b"] = LightData.BaseLightProperties.Color.z;
	j["light_intensity"] = LightData.BaseLightProperties.Intensity;

	j["light_atten_constant"] = LightData.AttenuationProperties.Constant;
	j["light_atten_linear"] = LightData.AttenuationProperties.Linear;
	j["light_atten_exp"] = LightData.AttenuationProperties.Exp;
}

void PointLightNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	LightData.BaseLightProperties.Color = nodeData.lightColor;
	LightData.BaseLightProperties.Intensity = nodeData.lightIntensity;
	LightData.AttenuationProperties.Constant = nodeData.lightAttenuation.x;
	LightData.AttenuationProperties.Linear = nodeData.lightAttenuation.y;
	LightData.AttenuationProperties.Exp = nodeData.lightAttenuation.z;
}

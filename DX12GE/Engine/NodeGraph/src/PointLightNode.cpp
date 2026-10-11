#include "../../Base/Singleton.h"
#include "../../Graphics/ResourceStorage.h"

PointLightNode::PointLightNode() : Node3D(), m_LightDataComponentId(-1)
{
	m_Type = NODE_TYPE_POINT_LIGHT;
	Rename("PointLightNode");
}

void PointLightNode::SetComponentId(uint32_t newId)
{
	m_LightDataComponentId = newId;
	GetLightComponent();
}

PointLightComponent* PointLightNode::GetLightComponent()
{
	PointLightComponent* light = ResourceStorage::GetPointLightComponent(m_LightDataComponentId);
	if (!light)
	{
		throw std::runtime_error("Point Light component id was invalid");
	}
	return light;
}

void PointLightNode::OnUpdate(const double& deltaTime)
{
	Node3D::OnUpdate(deltaTime);

	PointLightComponent* light = GetLightComponent();
	light->WorldPosition = m_WorldPositionCache;
}

Node3D* PointLightNode::Clone(Node3D* newParent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
	if (!cloneNode)
	{
		cloneNode = new PointLightNode();
	}

	Node3D::Clone(newParent, cloneChildrenRecursive, cloneNode);

	if (cloneNode && cloneNode->IsInsideTree())
	{
		PointLightNode* pLight = dynamic_cast<PointLightNode*>(cloneNode);

		PointLightComponent* cloneLight = pLight->GetLightComponent();
		PointLightComponent* light = GetLightComponent();

		cloneLight->BaseLightProperties = light->BaseLightProperties;
		cloneLight->AttenuationProperties = light->AttenuationProperties;
	}

	return cloneNode;
}

void PointLightNode::DrawDebug()
{
	Node3D::DrawDebug();
	PointLightComponent* light = GetLightComponent();
	Singleton::GetDebugRender()->DrawPoint(m_WorldPositionCache, 0.5f, light->BaseLightProperties.Color);
	Singleton::GetDebugRender()->DrawSphere(0.5f, light->BaseLightProperties.Color, m_WorldMatrixCache, 10);
}

void PointLightNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);

	PointLightComponent* light = GetLightComponent();
	
	j["light_color_r"] = light->BaseLightProperties.Color.x;
	j["light_color_g"] = light->BaseLightProperties.Color.y;
	j["light_color_b"] = light->BaseLightProperties.Color.z;
	j["light_intensity"] = light->BaseLightProperties.Intensity;

	j["light_atten_constant"] = light->AttenuationProperties.Constant;
	j["light_atten_linear"] = light->AttenuationProperties.Linear;
	j["light_atten_exp"] = light->AttenuationProperties.Exp;
}

void PointLightNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	PointLightComponent* light = GetLightComponent();

	light->BaseLightProperties.Color = nodeData.lightColor;
	light->BaseLightProperties.Intensity = nodeData.lightIntensity;
	light->AttenuationProperties.Constant = nodeData.lightAttenuation.x;
	light->AttenuationProperties.Linear = nodeData.lightAttenuation.y;
	light->AttenuationProperties.Exp = nodeData.lightAttenuation.z;
}
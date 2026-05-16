#include "../../Base/Singleton.h"
#include "../../Graphics/ResourceStorage.h"

PointLightNode::PointLightNode() : Node3D(), m_LightDataComponent(nullptr), m_LightDataComponentId(-1)
{
	m_Type = NODE_TYPE_POINT_LIGHT;
	Rename("PointLightNode");
}

void PointLightNode::SetComponentId(uint32_t newId)
{
	m_LightDataComponentId = newId;
	m_LightDataComponent = ResourceStorage::GetPointLightComponent(newId);
	if (!m_LightDataComponent)
	{
		throw std::runtime_error("Point Light component id was invalid");
	}
}

void PointLightNode::OnUpdate(const double& deltaTime)
{
	Node3D::OnUpdate(deltaTime);
	m_LightDataComponent->WorldPosition = m_WorldPositionCache;
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
		if (pLight->m_LightDataComponent && m_LightDataComponent)
		{
			pLight->m_LightDataComponent->BaseLightProperties = m_LightDataComponent->BaseLightProperties;
			pLight->m_LightDataComponent->AttenuationProperties = m_LightDataComponent->AttenuationProperties;
		}
	}

	return cloneNode;
}

void PointLightNode::DrawDebug()
{
	Node3D::DrawDebug();
	Singleton::GetDebugRender()->DrawPoint(m_WorldPositionCache, 0.5f, m_LightDataComponent->BaseLightProperties.Color);
	Singleton::GetDebugRender()->DrawSphere(0.5f, m_LightDataComponent->BaseLightProperties.Color, m_WorldMatrixCache, 10);
}

void PointLightNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);
	
	j["light_color_r"] = m_LightDataComponent->BaseLightProperties.Color.x;
	j["light_color_g"] = m_LightDataComponent->BaseLightProperties.Color.y;
	j["light_color_b"] = m_LightDataComponent->BaseLightProperties.Color.z;
	j["light_intensity"] = m_LightDataComponent->BaseLightProperties.Intensity;

	j["light_atten_constant"] = m_LightDataComponent->AttenuationProperties.Constant;
	j["light_atten_linear"] = m_LightDataComponent->AttenuationProperties.Linear;
	j["light_atten_exp"] = m_LightDataComponent->AttenuationProperties.Exp;
}

void PointLightNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	m_LightDataComponent->BaseLightProperties.Color = nodeData.lightColor;
	m_LightDataComponent->BaseLightProperties.Intensity = nodeData.lightIntensity;
	m_LightDataComponent->AttenuationProperties.Constant = nodeData.lightAttenuation.x;
	m_LightDataComponent->AttenuationProperties.Linear = nodeData.lightAttenuation.y;
	m_LightDataComponent->AttenuationProperties.Exp = nodeData.lightAttenuation.z;
}
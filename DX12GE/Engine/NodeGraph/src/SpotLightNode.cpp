#include "../../Base/Singleton.h"
#include "../../Graphics/ResourceStorage.h"

SpotLightNode::SpotLightNode() : Node3D(), m_LightDataComponent(nullptr), m_LightDataComponentId(-1)
{
	m_Type = NODE_TYPE_SPOT_LIGHT;
	Rename("SpotLightNode");
}

void SpotLightNode::SetComponentId(uint32_t newId)
{
	m_LightDataComponentId = newId;
	m_LightDataComponent = ResourceStorage::GetSpotLightComponent(newId);
	if (!m_LightDataComponent)
	{
		throw std::runtime_error("Spot Light component id was invalid");
	}
}

void SpotLightNode::OnUpdate(const double& deltaTime)
{
	Node3D::OnUpdate(deltaTime);
	m_LightDataComponent->WorldDirection = m_WorldDirectionCache;
	m_LightDataComponent->PointLightProperties.WorldPosition = m_WorldPositionCache;
}

Node3D* SpotLightNode::Clone(Node3D* newParent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
	if (!cloneNode)
	{
		cloneNode = new SpotLightNode();
	}

	Node3D::Clone(newParent, cloneChildrenRecursive, cloneNode);

	if (cloneNode && cloneNode->IsInsideTree())
	{
		SpotLightNode* sLight = dynamic_cast<SpotLightNode*>(cloneNode);
		if (sLight->m_LightDataComponent && m_LightDataComponent)
		{
			sLight->m_LightDataComponent->PointLightProperties = m_LightDataComponent->PointLightProperties;
			sLight->m_LightDataComponent->Cutoff = m_LightDataComponent->Cutoff;
		}
	}

	return cloneNode;
}

void SpotLightNode::DrawDebug()
{
	Node3D::DrawDebug();

	Singleton::GetDebugRender()->DrawCone(
		m_WorldPositionCache, m_WorldDirectionCache, acos(m_LightDataComponent->Cutoff) * 2.0f,
		{
			m_LightDataComponent->PointLightProperties.BaseLightProperties.Color.x,
			m_LightDataComponent->PointLightProperties.BaseLightProperties.Color.y,
			m_LightDataComponent->PointLightProperties.BaseLightProperties.Color.z
		}
	);
}

void SpotLightNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);
	
	j["light_color_r"] = m_LightDataComponent->PointLightProperties.BaseLightProperties.Color.x;
	j["light_color_g"] = m_LightDataComponent->PointLightProperties.BaseLightProperties.Color.y;
	j["light_color_b"] = m_LightDataComponent->PointLightProperties.BaseLightProperties.Color.z;
	j["light_intensity"] = m_LightDataComponent->PointLightProperties.BaseLightProperties.Intensity;

	j["light_atten_constant"] = m_LightDataComponent->PointLightProperties.AttenuationProperties.Constant;
	j["light_atten_linear"] = m_LightDataComponent->PointLightProperties.AttenuationProperties.Linear;
	j["light_atten_exp"] = m_LightDataComponent->PointLightProperties.AttenuationProperties.Exp;

	j["light_cutoff"] = m_LightDataComponent->Cutoff;
}

void SpotLightNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	m_LightDataComponent->PointLightProperties.BaseLightProperties.Color = nodeData.lightColor;
	m_LightDataComponent->PointLightProperties.BaseLightProperties.Intensity = nodeData.lightIntensity;
	m_LightDataComponent->PointLightProperties.AttenuationProperties.Constant = nodeData.lightAttenuation.x;
	m_LightDataComponent->PointLightProperties.AttenuationProperties.Linear = nodeData.lightAttenuation.y;
	m_LightDataComponent->PointLightProperties.AttenuationProperties.Exp = nodeData.lightAttenuation.z;
	m_LightDataComponent->Cutoff = nodeData.lightCutoff;
}
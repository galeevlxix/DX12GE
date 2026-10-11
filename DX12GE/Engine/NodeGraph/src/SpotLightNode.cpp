#include "../../Base/Singleton.h"
#include "../../Graphics/ResourceStorage.h"

SpotLightNode::SpotLightNode() : Node3D(), m_LightDataComponentId(-1)
{
	m_Type = NODE_TYPE_SPOT_LIGHT;
	Rename("SpotLightNode");
}

void SpotLightNode::SetComponentId(uint32_t newId)
{
	m_LightDataComponentId = newId;
	GetLightComponent();
}

SpotLightComponent* SpotLightNode::GetLightComponent()
{
	SpotLightComponent* light = ResourceStorage::GetSpotLightComponent(m_LightDataComponentId);
	if (!light)
	{
		throw std::runtime_error("Spot Light component id was invalid");
	}
	return light;
}

void SpotLightNode::OnUpdate(const double& deltaTime)
{
	Node3D::OnUpdate(deltaTime);

	SpotLightComponent* light = GetLightComponent();
	light->WorldDirection = m_WorldDirectionCache;
	light->PointLightProperties.WorldPosition = m_WorldPositionCache;
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

		SpotLightComponent* cloneLight = sLight->GetLightComponent();
		SpotLightComponent* light = GetLightComponent();

		cloneLight->PointLightProperties = light->PointLightProperties;
		cloneLight->Cutoff = light->Cutoff;
	}

	return cloneNode;
}

void SpotLightNode::DrawDebug()
{
	Node3D::DrawDebug();

	SpotLightComponent* light = GetLightComponent();

	Singleton::GetDebugRender()->DrawCone(
		m_WorldPositionCache, m_WorldDirectionCache, acos(light->Cutoff) * 2.0f,
		{
			light->PointLightProperties.BaseLightProperties.Color.x,
			light->PointLightProperties.BaseLightProperties.Color.y,
			light->PointLightProperties.BaseLightProperties.Color.z
		}
	);
}

void SpotLightNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);

	SpotLightComponent* light = GetLightComponent();
	
	j["light_color_r"] = light->PointLightProperties.BaseLightProperties.Color.x;
	j["light_color_g"] = light->PointLightProperties.BaseLightProperties.Color.y;
	j["light_color_b"] = light->PointLightProperties.BaseLightProperties.Color.z;
	j["light_intensity"] = light->PointLightProperties.BaseLightProperties.Intensity;

	j["light_atten_constant"] = light->PointLightProperties.AttenuationProperties.Constant;
	j["light_atten_linear"] = light->PointLightProperties.AttenuationProperties.Linear;
	j["light_atten_exp"] = light->PointLightProperties.AttenuationProperties.Exp;

	j["light_cutoff"] = light->Cutoff;
}

void SpotLightNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	SpotLightComponent* light = GetLightComponent();

	light->PointLightProperties.BaseLightProperties.Color = nodeData.lightColor;
	light->PointLightProperties.BaseLightProperties.Intensity = nodeData.lightIntensity;
	light->PointLightProperties.AttenuationProperties.Constant = nodeData.lightAttenuation.x;
	light->PointLightProperties.AttenuationProperties.Linear = nodeData.lightAttenuation.y;
	light->PointLightProperties.AttenuationProperties.Exp = nodeData.lightAttenuation.z;
	light->Cutoff = nodeData.lightCutoff;
}
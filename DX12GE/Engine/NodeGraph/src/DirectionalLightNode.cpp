#include "../../Base/Singleton.h"

DirectionalLightNode::DirectionalLightNode() : Node3D()
{
	m_Type = NODE_TYPE_DIRECTIONAL_LIGHT;
	LightData = DirectionalLightComponent();
	Rename("DirectionalLightNode");
}

void DirectionalLightNode::OnUpdate(const double& deltaTime)
{
	Node3D::OnUpdate(deltaTime);
	LightData.Direction = Vector4(m_WorldDirectionCache);
}

Node3D* DirectionalLightNode::Clone(Node3D* newParrent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
	if (!cloneNode)
	{
		cloneNode = new DirectionalLightNode();
	}

	Node3D::Clone(newParrent, cloneChildrenRecursive, cloneNode);

	if (cloneNode)
	{
		DirectionalLightNode* dirLight = dynamic_cast<DirectionalLightNode*>(cloneNode);
		dirLight->LightData.BaseLightProperties = LightData.BaseLightProperties;
	}

	return cloneNode;
}

void DirectionalLightNode::DrawDebug()
{
	Node3D::DrawDebug();
	Singleton::GetDebugRender()->DrawArrow(
		m_WorldPositionCache,
		m_WorldPositionCache + m_WorldDirectionCache,
		LightData.BaseLightProperties.Color,
		abs(m_WorldDirectionCache.y) == 1.0f ? Vector3::UnitX : Vector3::UnitY);
}

void DirectionalLightNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);
	
	j["light_color_r"] = LightData.BaseLightProperties.Color.x;
	j["light_color_g"] = LightData.BaseLightProperties.Color.y;
	j["light_color_b"] = LightData.BaseLightProperties.Color.z;
	j["light_intensity"] = LightData.BaseLightProperties.Intensity;

	if (IsCurrent())
	{
		j["is_current"] = true;
	}
}

void DirectionalLightNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	LightData.BaseLightProperties.Color = nodeData.lightColor;
	LightData.BaseLightProperties.Intensity = nodeData.lightIntensity;

	if (nodeData.isCurrent)
	{
		SetCurrent();
	}
}

void DirectionalLightNode::SetCurrent()
{
	if (IsInsideTree())
	{
		Singleton::GetNodeGraph()->m_CurrentDirectionalLight = this;
	}
	else
	{
		printf("Внимание! Невозможно сделать DirectionalLightNode::%s активным! Узел не находится в дереве сцены!\n", m_Name.c_str());
	}
}

bool DirectionalLightNode::IsCurrent()
{
	return Singleton::GetNodeGraph()->m_CurrentDirectionalLight == this;
}
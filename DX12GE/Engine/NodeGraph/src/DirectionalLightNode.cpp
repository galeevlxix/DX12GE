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

void DirectionalLightNode::Clone(Node3D* cloneNode, Node3D* newParrent, bool cloneChildrenRecursive)
{
	if (!cloneNode)
	{
		cloneNode = new DirectionalLightNode();
	}

	Node3D::Clone(cloneNode, newParrent, cloneChildrenRecursive);

	if (cloneNode)
	{
		DirectionalLightNode* dirLight = dynamic_cast<DirectionalLightNode*>(cloneNode);
		dirLight->LightData.BaseLightProperties = LightData.BaseLightProperties;
	}
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

void DirectionalLightNode::SetCurrent()
{
	if (IsInsideTree())
	{
		Singleton::GetNodeGraph()->m_CurrentDirectionalLight = this;
	}
}

bool DirectionalLightNode::IsCurrent()
{
	return Singleton::GetNodeGraph()->m_CurrentDirectionalLight == this;
}
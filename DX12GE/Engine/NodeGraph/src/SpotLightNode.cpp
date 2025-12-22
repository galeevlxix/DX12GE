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

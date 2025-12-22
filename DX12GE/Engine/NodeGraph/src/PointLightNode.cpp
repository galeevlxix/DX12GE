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

void PointLightNode::Clone(Node3D* cloneNode, Node3D* newParrent, bool cloneChildrenRecursive)
{
	if (!cloneNode)
	{
		cloneNode = new PointLightNode();
	}

	Node3D::Clone(cloneNode, newParrent, cloneChildrenRecursive);

	if (cloneNode)
	{
		PointLightNode* pLight = dynamic_cast<PointLightNode*>(cloneNode);
		pLight->LightData = LightData;
	}
}

void PointLightNode::DrawDebug()
{
	Node3D::DrawDebug();
	Singleton::GetDebugRender()->DrawPoint(m_WorldPositionCache, 0.5f, LightData.BaseLightProperties.Color);
}

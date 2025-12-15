#include "../../Base/Singleton.h"

PointLightNode::PointLightNode() : Node3D()
{
	LightData = PointLightComponent();
	Rename("PointLightNode");
}

void PointLightNode::OnUpdate(const double& deltaTime)
{
	bool wasDirty = Transform.IsCacheDirty();
	Node3D::OnUpdate(deltaTime);
	if (wasDirty)
	{
		LightData.WorldPosition = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
		LightData.WorldPosition = DirectX::SimpleMath::Vector3::Transform(LightData.WorldPosition, m_WorldMatrixCache);
	}
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

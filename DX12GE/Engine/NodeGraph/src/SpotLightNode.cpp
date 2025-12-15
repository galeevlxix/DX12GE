#include "../../Base/Singleton.h"

SpotLightNode::SpotLightNode() : Node3D()
{
	LightData = SpotLightComponent();
	Rename("SpotLightNode");
}

void SpotLightNode::OnUpdate(const double& deltaTime)
{
	bool wasDirty = Transform.IsCacheDirty();
	Node3D::OnUpdate(deltaTime);
	if (wasDirty)
	{
		DirectX::SimpleMath::Vector3 start = DirectX::SimpleMath::Vector3::Zero;
		DirectX::SimpleMath::Vector3 end = DirectX::SimpleMath::Vector3::Backward;

		start = DirectX::SimpleMath::Vector3::Transform(start, m_WorldMatrixCache);
		end = DirectX::SimpleMath::Vector3::Transform(end, m_WorldMatrixCache);

		LightData.Direction = end - start;
		LightData.Direction.Normalize();

		LightData.PointLightProperties.WorldPosition = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
		LightData.PointLightProperties.WorldPosition = DirectX::SimpleMath::Vector3::Transform(LightData.PointLightProperties.WorldPosition, m_WorldMatrixCache);
	}
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

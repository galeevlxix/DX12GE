#include "../DirectionalLightNode.h"

DirectionalLightNode::DirectionalLightNode() : Node3D()
{
	Color = Vector3(1.0f, 1.0f, 1.0f);
	Intensity = 0.6f;
	Rename("DirectionalLightNode");
}

void DirectionalLightNode::OnUpdate(const double& deltaTime)
{
	bool wasDirty = Transform.IsCacheDirty();
	Node3D::OnUpdate(deltaTime);
	if (wasDirty)
	{
		DirectX::SimpleMath::Vector3 start = DirectX::SimpleMath::Vector3::Zero;
		DirectX::SimpleMath::Vector3 end = DirectX::SimpleMath::Vector3::Backward;

		start = DirectX::SimpleMath::Vector3::Transform(start, m_WorldMatrixCache);
		end = DirectX::SimpleMath::Vector3::Transform(end, m_WorldMatrixCache);

		m_DirectionCache = end - start;
		m_DirectionCache.Normalize();
	}
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
		dirLight->Color = Color;
		dirLight->Intensity = Intensity;
	}
}

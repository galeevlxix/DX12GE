#pragma once

#include "Node3D.h"

// Направленный источник света
// Может быть только один такой узел в дереве сцены
class DirectionalLightNode : public Node3D
{
private:
	DirectX::SimpleMath::Vector3 m_DirectionCache;

public:
	DirectX::SimpleMath::Vector3 Color;
	float Intensity;

	DirectionalLightNode();

	virtual const std::string GetType() override { return "DirectionalLightNode"; }

	virtual void OnUpdate(const double& deltaTime) override;

	const DirectX::SimpleMath::Vector3& GetWorldDirection() { return m_DirectionCache; }

	virtual void Clone(Node3D* cloneNode, Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false) override;
};
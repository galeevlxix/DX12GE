#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

// Точечный источник света
class PointLightNode : public Node3D
{
public:
	PointLightComponent LightData;

	PointLightNode();

	virtual void OnUpdate(const double& deltaTime) override;

	virtual void Clone(Node3D* cloneNode, Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false) override;

	virtual void DrawDebug() override;
};
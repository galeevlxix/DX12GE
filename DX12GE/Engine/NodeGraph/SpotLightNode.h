#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

class SpotLightNode : public Node3D
{
public:	
	SpotLightComponent LightData;

	SpotLightNode();

	virtual const std::string GetType() override { return "SpotLightNode"; }

	virtual void OnUpdate(const double& deltaTime) override;

	virtual void Clone(Node3D* cloneNode, Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false) override;
};
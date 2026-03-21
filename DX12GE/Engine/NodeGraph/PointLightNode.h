#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

/// \brief Point light source node class.
class PointLightNode : public Node3D
{
public:
	/// \brief Parameters of the point light.
	PointLightComponent LightData;

	PointLightNode();

	virtual void OnUpdate(const double& deltaTime) override;

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;
};
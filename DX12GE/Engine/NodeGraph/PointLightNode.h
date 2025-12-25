#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

// Класс узла точечного источника света
class PointLightNode : public Node3D
{
public:
	PointLightComponent LightData;

	PointLightNode();

	virtual void OnUpdate(const double& deltaTime) override;

	virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;
};
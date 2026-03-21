#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

/// \brief Class of directional light source node.
/// \note Only one such node in the scene tree can be active. 
class DirectionalLightNode : public Node3D
{
public:
	/// \brief Parameters of the light source.
	DirectionalLightComponent LightData;

	DirectionalLightNode();

	virtual void OnUpdate(const double& deltaTime) override;

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;

	/// \brief Checks whether this directional light is active in the scene.
	/// \return Returns true if this light is current. Returns false otherwise.
	bool IsCurrent();
};
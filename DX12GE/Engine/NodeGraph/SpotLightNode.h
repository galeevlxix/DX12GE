#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

/// \brief Class of spotlight light source node.
class SpotLightNode : public Node3D
{
	/// \brief Parameters of the spot light.
	SpotLightComponent* m_LightDataComponent;
	uint32_t m_LightDataComponentId;

public:	

	SpotLightNode();

	void SetComponentId(uint32_t newId);

	uint32_t GetComponentId() { return m_LightDataComponentId; }

	virtual void OnUpdate(const double& deltaTime) override;

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;
};
#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

/// \brief Class of scene environment node (ambient light, fog, post-processing effects, etc.).
/// \note Only one such node in the scene tree can be active. 
class EnvironmentNode : public Node3D
{
public:
	/// \brief Parameters of the ambient light.
	BaseLightComponent AmbientLightData;

	/// \brief The fog is on.
	bool FogEnabled;
	/// \brief Color of the fog.
	DirectX::SimpleMath::Vector3 FogColor;
	/// \brief Start distance of the fog.
	float FogStart;
	/// \brief The distance at which fog completely covers objects.
	float FogDistance;

	/// \brief Maximum length of the reflected ray.
	float SSRMaxDistance;
	/// \brief Length of the reflected ray's traversal step. Affects reflection quality.
	float SSRStepLength;
	/// \brief Thickness of the reflected ray's traversal step. Affects reflection quality.
	float SSRThickness;

	EnvironmentNode();

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;

	/// \brief Checks whether this environment is active in the scene.
	/// \return Returns true if this environment is current. Returns false otherwise.
	bool IsCurrent();
};
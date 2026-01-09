#pragma once

#include "FirstPersonPlayerNode.h"

/// \brief Player node class for third-person camera control.
/// \note Only one such node in the scene tree can be active. 
class ThirdPersonPlayerNode : public FirstPersonPlayerNode
{
public:
	/// \brief Minimum distance between the camera and the player.
	float MinFlyRadius;
	/// \brief Maximum distance between the camera and the player.
	float MaxFlyRadius;
	/// \brief The point around which the camera rotates (in the player's local space).
	Vector3 CameraAnchor;

protected:
	/// \brief Current distance between the camera and the player.
	float m_FlyRadius;

public:
	ThirdPersonPlayerNode();

	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath) override;
	
	virtual void OnUpdate(const double& deltaTime) override;

	virtual void Destroy(bool keepComponent = true) override;

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
};
#pragma once

#include "Object3DNode.h"
#include "CameraNode.h"
#include "PhysicalObjectNode.h"
#include "../Base/InputSystem.h"

/// \brief Player node class for first-person camera control
/// \note Only one such node in the scene tree can be active. 
class FirstPersonPlayerNode : public PhysicalObjectNode
{
public:
	/// \brief Mouse movement sensitivity
	float MouseSensitivity;
	/// \brief Mouse wheel movement sensitivity
	float WheelSensitivity;

	/// \brief Slow movement speed
	float MinMovementSpeed;
	/// \brief Normal movement speed
	float NormalMovementSpeed;
	/// \brief Fast movement speed
	float MaxMovementSpeed;
	
	float JumpImpulse;

protected:
	/// \brief Current camera attached to the player.
	/// \warning Direct modification of the this parameter is not permitted. Use the SetCamera method instead.
	CameraNode* m_Camera;

	/// \brief Tracking pressed keys and mouse buttons.
	/// \warning Direct modification of the this parameter is not permitted. This parameter is automatically updated in OnKeyPressed, OnKeyReleased, OnMouseButtonPressed and OnMouseButtonPressed methods.
	PressedKeysAndButtons m_PressedInputs;

	/// \brief Current movement speed.
	/// \warning Direct modification of the this parameter is not permitted. This parameter is automatically updated in OnUpdate method.
	float m_CurrentMovementSpeed;

	float m_angle_h;
	float m_angle_v;
	int m_prevX;
	int m_prevY;
	int m_dx;
	int m_dy;

public:
	FirstPersonPlayerNode();

	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath) override;
	
	virtual void OnUpdate(const double& deltaTime) override;

	virtual void Destroy(bool keepComponent = true) override;

	virtual bool AddChild(Node3D* node) override;

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;
	
	/// \brief Checks whether this player is active in the scene.
	/// \return Returns true if this player is current. Returns false otherwise.
	bool IsCurrent();

	/// \brief Makes the camera the active camera for this player if the camera node is a child of this node. Makes the camera the active camera for the scene if this player node is active.
	void SetCamera(CameraNode* camera);

	/// \brief Returns the current camera of the player
	CameraNode* GetCamera() { return m_Camera; }

	virtual void OnKeyPressed(KeyEventArgs& e) override;
	virtual void OnKeyReleased(KeyEventArgs& e) override;
	virtual void OnMouseMoved(MouseMotionEventArgs& e) override;
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
};

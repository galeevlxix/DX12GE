#pragma once

#include "Object3DNode.h"
#include "CameraNode.h"
#include "../Base/InputSystem.h"

//  ласс узла игрока дл€ управлени€ камерой от первого лица
// “олько один такой узел в дереве сцены может быть активным 
class FirstPersonPlayerNode : public Object3DNode
{
public:
	float MouseSensitivity;
	float WheelSensitivity;

	float MinMovementSpeed;
	float NormalMovementSpeed;
	float MaxMovementSpeed;

protected:
	CameraNode* m_Camera;

	PressedKeysAndButtons m_PressedInputs;

	float m_CurrentMovementSpeed;

	float m_angle_h;
	float m_angle_v;
	int m_prevX;
	int m_prevY;
	int m_dx;
	int m_dy;

public:
	FirstPersonPlayerNode();

	virtual void OnUpdate(const double& deltaTime) override;

	virtual void Destroy(bool keepComponent = true) override;

	virtual bool AddChild(Node3D* node) override;

	virtual void Clone(Node3D* cloneNode, Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false) override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;
	bool IsCurrent();

	// ƒелает камеру активной камерой этого игрока, если узел камеры - дочерний данного узла
	// ƒелает камеру активной камерой сцены, если данный узел игрока активен
	void SetCamera(CameraNode* camera);

	// ¬озвращает активную камеру данного узла
	CameraNode* GetCamera() { return m_Camera; }

	virtual void OnKeyPressed(KeyEventArgs& e) override;
	virtual void OnKeyReleased(KeyEventArgs& e) override;
	virtual void OnMouseMoved(MouseMotionEventArgs& e) override;
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
};

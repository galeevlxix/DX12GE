#pragma once

#include "CameraNode.h"
#include "Object3DNode.h"
#include "../Base/InputSystem.h"

//  ласс игрока дл€ управлени€ камерой от первого лица
// “олько один такой узел в дереве сцены может быть активным 
class FirstPersonPlayerNode : public Object3DNode
{
protected:
	CameraNode* m_Camera;

	PressedKeysAndButtons m_PressedInputs;

	float m_MovementSpeed;
	const float m_MinMovementSpeed = 2.0f;
	const float m_NormalMovementSpeed = 4.0f;
	const float m_MaxMovementSpeed = 8.0f;

	float m_Sensitivity;
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

	void SetCurrent();
	bool IsCurrent();

	// ѕрикрепл€ет камеру, если узел камеры - дочерний данного узла
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

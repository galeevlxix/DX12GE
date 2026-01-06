#pragma once

#include "Object3DNode.h"
#include "CameraNode.h"
#include "PhysicalObjectNode.h"
#include "../Base/InputSystem.h"

// ����� ���� ������ ��� ���������� ������� �� ������� ����
// ������ ���� ����� ���� � ������ ����� ����� ���� �������� 
class FirstPersonPlayerNode : public PhysicalObjectNode
{
public:
	float MouseSensitivity;
	float WheelSensitivity;

	float MinMovementSpeed;
	float NormalMovementSpeed;
	float MaxMovementSpeed;
	
	float JumpImpulse;

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

	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath, const std::string& nodePath) override;
	
	virtual void OnUpdate(const double& deltaTime) override;

	virtual void Destroy(bool keepComponent = true) override;

	virtual bool AddChild(Node3D* node) override;

	virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;
	bool IsCurrent();

	// ������ ������ �������� ������� ����� ������, ���� ���� ������ - �������� ������� ����
	// ������ ������ �������� ������� �����, ���� ������ ���� ������ �������
	void SetCamera(CameraNode* camera);

	// ���������� �������� ������ ������� ����
	CameraNode* GetCamera() { return m_Camera; }

	virtual void OnKeyPressed(KeyEventArgs& e) override;
	virtual void OnKeyReleased(KeyEventArgs& e) override;
	virtual void OnMouseMoved(MouseMotionEventArgs& e) override;
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
};

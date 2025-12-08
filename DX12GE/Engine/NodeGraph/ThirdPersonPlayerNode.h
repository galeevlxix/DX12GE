#pragma once

#include "Object3DNode.h"
#include "../Base/SimpleMath.h"
#include "../Graphics/Camera.h"

using namespace DirectX::SimpleMath;

class ThirdPersonPlayerNode : public Object3DNode
{
public:
	Vector3 m_Direction;

private:
	float m_Speed = 0.0f;
	float m_Sensitivity = 0.0075f;
	float m_angle_h = 0.0f;
	float m_angle_v = 0.0f;
	int m_prevX = 0;
	int m_prevY = 0;
	int m_dx = 0;
	int m_dy = 0;
	float m_FlyRadius = 15.0f;

	Camera* m_Camera;

public:
	ThirdPersonPlayerNode();

	virtual const std::string GetType() override { return "ThirdPersonPlayerNode"; }

	virtual void OnUpdate(const double& deltaTime) override;
	virtual void Destroy(bool keepComponent = true) override;

	void SetCamera(Camera* camera);

	virtual void OnKeyPressed(KeyEventArgs& e) override;
	virtual void OnKeyReleased(KeyEventArgs& e) override;
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
	virtual void OnMouseMoved(MouseMotionEventArgs& e) override;
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;


private:
	// TESTING MODE

	bool m_test_Enabled = false;
	float m_test_MoveStep = 0.5f;

	Vector2 m_test_MapMin = Vector2(-150.0f, -30.0f);
	Vector2 m_test_MapMax = Vector2(30.0f, 90.0f);

	Vector3 m_test_PlayerPosInTest = Vector3(-90.0f, 8.0f, 24.0f);
	Vector3 m_test_PlayerRotInTest = Vector3(0.0f, 1.57f, 0.0f);
	Vector3 m_test_InitPos = Vector3();
	Vector3 m_test_InitTar = Vector3();
	Vector3 m_test_InitPosPlayer = Vector3();
	float m_test_InitRotYPlayer = 0.0f;

	struct TestPhase
	{
		bool enable = false;
		Vector3 start = Vector3();
		Vector3 target = Vector3();
		Vector3 move_target = Vector3();

		TestPhase(Vector3 s, Vector3 t, Vector3 m) : start(s), target(t), move_target(m) {}
	};

	const static int m_test_MaxPhases = 6;
	TestPhase m_test_Phases[m_test_MaxPhases] =
	{
		TestPhase(Vector3(m_test_MapMax.x - 0.1f,	10.0f,	17.0f),		Vector3(-1.0f,	0.0f,	0.0f),		Vector3(-1.0f,	0.0f,	0.0f)),
		TestPhase(Vector3(m_test_MapMin.x + 0.1f,	10.0f,	0.0f),		Vector3(1.0f,	-0.2f,	0.0f),		Vector3(1.0f,	0.0f,	0.0f)),
		TestPhase(Vector3(m_test_MapMin.x + 0.1f,	12.0f,	42.0f),		Vector3(1.0f,	-0.15f, 0.0f),		Vector3(1.0f,	0.0f,	0.0f)),
		TestPhase(Vector3(-42.0f, 10.0f,	m_test_MapMax.y - 0.1f),	Vector3(0.3f,	-0.2f,	-1.0f),		Vector3(0.0f,	0.0f,	-1.0f)),
		TestPhase(Vector3(0.0f, 12.0f,	m_test_MapMin.y + 0.1f),		Vector3(0.0f,	0.0f,	1.0f),		Vector3(0.0f,	0.0f,	1.0f)),
		TestPhase(Vector3(-93.0f, 12.0f, -5.0f),						Vector3(-0.3f,	0.0f,	1.0f),		Vector3(0.0f,	0.0f,	1.0f)),
	};

	void TestProcess();

public:
	void StartTest();
	bool IsTesting();
};
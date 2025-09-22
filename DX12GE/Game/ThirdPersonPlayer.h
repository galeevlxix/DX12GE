#pragma once
#include "../Engine/Graphics/Object3DEntity.h"
#include "../Engine/Base/Events.h"
#include "../Engine/Graphics/Camera.h"

class ThirdPersonPlayer : public Object3DEntity
{
	// MOVING & CAMERA CONTROL
public:
	Vector3 m_Direction;

private:
	float m_Speed;
	float m_Sensitivity = 0.0075f;
	float m_angle_h = 0.0f;
	float m_angle_v = 0.0f;
	int m_prevX;
	int m_prevY;
	int m_dx = 0;
	int m_dy = 0;
	float m_FlyRadius = 15.0f;

	Camera* m_Camera;

public:	

	virtual void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath) override;
	virtual void OnUpdate(const double& deltaTime) override;

	void SetCamera(Camera* camera);

	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e);
	void OnMouseWheel(MouseWheelEventArgs& e);
	void OnMouseMoved(MouseMotionEventArgs& e);
	void OnMouseButtonPressed(MouseButtonEventArgs& e);
	void OnMouseButtonReleased(MouseButtonEventArgs& e);

	virtual void Destroy() override;

private:
	// TESTING MODE

	bool m_test_Enabled = false;
	float m_test_MoveStep = 0.5;

	Vector2 m_test_MapMin = Vector2(-150.0f, -30.0f);
	Vector2 m_test_MapMax = Vector2(30.0f, 90.0f);

	Vector3 m_test_PlayerPosInTest = Vector3(-90.0f, 8.0f, 24.0f);
	Vector3 m_test_PlayerRotInTest = Vector3(0.0f, 1.57f, 0.0f);
	Vector3 m_test_InitPos;
	Vector3 m_test_InitTar;
	Vector3 m_test_InitPosPlayer;
	float m_test_InitRotYPlayer;

	struct TestPhase
	{
		bool enable = false;
		Vector3 start;
		Vector3 target;
		Vector3 move_target;

		TestPhase(Vector3 s, Vector3 t, Vector3 m) : start(s), target(t), move_target(m) {}
	};

	const static int m_test_MaxPhases = 6;
	TestPhase m_test_Phases[m_test_MaxPhases] =
	{
		TestPhase(Vector3(m_test_MapMax.x - 0.1f,	10.0f,	17.0f),				Vector3(-1.0f,	0.0f,	0.0f),		Vector3(-1.0f,	0.0f,	0.0f)),
		TestPhase(Vector3(m_test_MapMin.x + 0.1f,	10.0f,	0.0f),				Vector3(1.0f,	-0.2f,	0.0f),		Vector3(1.0f,	0.0f,	0.0f)),
		TestPhase(Vector3(m_test_MapMin.x + 0.1f,	12.0f,	42.0f),				Vector3(1.0f,	-0.15f, 0.0f),		Vector3(1.0f,	0.0f,	0.0f)),
		TestPhase(Vector3(-42.0f,			10.0f,	m_test_MapMax.y - 0.1f),	Vector3(0.3f,	-0.2f,	-1.0f),		Vector3(0.0f,	0.0f,	-1.0f)),
		TestPhase(Vector3(0.0f,				12.0f,	m_test_MapMin.y + 0.1f),	Vector3(0.0f,	0.0f,	1.0f),		Vector3(0.0f,	0.0f,	1.0f)),
		TestPhase(Vector3(-93.0f,			12.0f,	-5.0f),						Vector3(-0.3f,	0.0f,	1.0f),		Vector3(0.0f,	0.0f,	1.0f)),
	};

	void TestProcess();

public:
	void StartTest();
	bool IsTesting();
};
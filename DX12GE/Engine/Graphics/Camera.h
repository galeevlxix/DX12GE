#pragma once
#include "../Base/Events.h"
#include "DirectXMath.h"
#include "../../Game/Player.h"

using namespace DirectX;

struct PressedKeyMonitor
{
	bool W = false;
	bool S = false;
	bool A = false;
	bool D = false;
	bool Q = false;
	bool E = false;

	bool Shift = false;
	bool Ctrl = false;

	bool LBC = false;
	bool RBC = false;
	bool MBC = false;
};

const static float slowSpeed = 6.0f;
const static float normalSpeed = 12.0f;
const static float fastSpeed = 24.0f;

class Camera
{
private:
	float speed = normalSpeed;

	float sensitivity = 0.0075f;
	float angle_h = 0.0f;
	float angle_v = 0.0f;
	int prevX;
	int prevY;
	int dx = 0;
	int dy = 0;

	float flyRadius = 15.0f;
	PressedKeyMonitor monitor;
	
	bool testMode = false;
	float step = 0.5;
	
	Vector2 MapMin = Vector2(-150.0, -30.0);
	Vector2 MapMax = Vector2(30.0, 90.0);
	
	Vector3 playerPosInTest = Vector3(-90.0, 8.0, 24.0);
	Vector3 playerRotInTest = Vector3(0.0, 1.57, 0.0);
	Vector3 initPos;
	Vector3 initTar; 
	Vector3 initPosPlayer;
	float initRotYPlayer;

	struct TestPhase
	{
		bool enable = false;
		Vector3 start;
		Vector3 target;
		Vector3 move_target;

		TestPhase(Vector3 s, Vector3 t, Vector3 m) : start(s), target(t), move_target(m) { }
	};

	const static int max_phases = 6;
	TestPhase phases[max_phases] =
	{
		TestPhase(Vector3(MapMax.x - 0.1, 10.0, 17.0),	Vector3(-1, 0, 0),		Vector3(-1, 0, 0)),
		TestPhase(Vector3(MapMin.x + 0.1, 10.0, 0.0),	Vector3(1, -0.2, 0),	Vector3(1, 0, 0)),
		TestPhase(Vector3(MapMin.x + 0.1, 12.0, 42.0),	Vector3(1, -0.15, 0),	Vector3(1, 0, 0)),
		TestPhase(Vector3(-42.0, 10.0, MapMax.y - 0.1),	Vector3(0.3, -0.2, -1),		Vector3(0, 0, -1)),
		TestPhase(Vector3(0.0, 12.0, MapMin.y + 0.1),	Vector3(0, 0, 1),		Vector3(0, 0, 1)),
		TestPhase(Vector3(-93.0, 12.0, -5.0),			Vector3(-0.3, 0, 1),		Vector3(0, 0, 1)),
	};

	void TestProcess();

public:
	XMVECTOR Position;
	XMVECTOR Target;  
	XMVECTOR Up;
	
	float Fov;
	float Ratio;
	float ZNear;
	float ZFar;

	Player* player;

	void OnLoad(
		Player* l_player, 
		XMVECTOR position = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f),
		XMVECTOR target = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
		float fov = 90.0f, 
		float ratio = 1.0f, 
		float zNear = 0.1f, 
		float zFar = 300.0);

	void OnUpdate(float deltaTime);
	XMMATRIX GetViewProjMatrix();
	XMMATRIX GetProjMatrix();

	void OnMouseWheel(MouseWheelEventArgs& e);
	void OnMouseMoved(MouseMotionEventArgs& e);
	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e);
	void OnMouseButtonPressed(MouseButtonEventArgs& e);
	void OnMouseButtonReleased(MouseButtonEventArgs& e);

	void StartTest();
	bool IsTesting();
};
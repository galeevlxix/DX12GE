#pragma once
#include "../Engine/Vector3.h"
#include "DirectXMath.h"
#include "../Engine/Events.h"

#include "Player.h"

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

const static float slowSpeed = 3;
const static float normalSpeed = 6;
const static float fastSpeed = 12;

class Camera
{
private:
	float speed = normalSpeed;
	
	float sensitivity = 0.01;
	float angle_h = 0.0;
	float angle_v = 0.0;
	float prevX;
	float prevY;
	float dx = 0.0;
	float startFov;

	PressedKeyMonitor monitor;
public:
	XMVECTOR Position = XMVectorSet(0, 0, -5, 1);
	XMVECTOR Target = XMVectorSet(0, 0, 1, 1);  
	XMVECTOR Up = XMVectorSet(0, 1, 0, 0);

	
	float Fov = 60.0f;
	float Ratio = 1.0f;
	float ZNear = 0.1f;
	float ZFar = 300.0f;

	Player* player;

	void OnLoad(XMVECTOR position, XMVECTOR target, XMVECTOR up, float fov, float ratio, float zNear, float zFar, Player* l_player);
	void OnUpdate(float deltaTime);
	XMMATRIX GetViewProjMatrix();

	void OnMouseWheel(MouseWheelEventArgs& e);
	void OnMouseMoved(MouseMotionEventArgs& e);
	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e);
	void OnMouseButtonPressed(MouseButtonEventArgs& e);
	void OnMouseButtonReleased(MouseButtonEventArgs& e);
};
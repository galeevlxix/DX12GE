#pragma once
#include "DirectXMath.h"
#include "../Engine/Events.h"

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

const static float slowSpeed = 10;
const static float normalSpeed = 25;
const static float fastSpeed = 50;

class Camera
{
private:
	float speed = normalSpeed;
	
	float sensitivity = 0.07;
	float angle_h = 0.0;
	float angle_v = 0.0;
	float prevX;
	float prevY;

	PressedKeyMonitor monitor;
public:
	XMVECTOR Position = XMVectorSet(0, 0, -5, 1);
	XMVECTOR Target = XMVectorSet(0, 0, 1, 1);  
	XMVECTOR Up = XMVectorSet(0, 1, 0, 0);

	float Fov = 60.0f;
	float Ratio = 1.0f;
	float ZNear = 0.1f;
	float ZFar = 300.0f;

	void OnLoad(XMVECTOR position, XMVECTOR target, XMVECTOR up, float fov, float ratio, float zNear, float zFar);
	void OnUpdate(float deltaTime);
	XMMATRIX GetViewProjMatrix();

	void OnMouseWheel(MouseWheelEventArgs& e);
	void OnMouseMoved(MouseMotionEventArgs& e);
	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e);
	void OnMouseButtonPressed(MouseButtonEventArgs& e);
	void OnMouseButtonReleased(MouseButtonEventArgs& e);
};
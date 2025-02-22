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
};

const static float slowSpeed = 0.001;
const static float normalSpeed = 0.0025;
const static float fastSpeed = 0.005;

class Camera
{
private:
	float angle_h = 0.0;
	float angle_v = 0.0;

	float speed = normalSpeed;
	float sensitivity = 0.05;

	bool RBC = false;

	float prevX;
	float prevY;

	PressedKeyMonitor monitor;
	void Render();
public:
	XMVECTOR Position = XMVectorSet(0, 0, -5, 1);
	XMVECTOR Target = XMVectorSet(0, 0, 1, 1);  
	XMVECTOR Up = XMVectorSet(0, 1, 0, 0);

	float Fov = 60.0f;
	float Ratio = 1.0f;
	float ZNear = 0.1f;
	float ZFar = 100.0f;

	void OnLoad(XMVECTOR position, XMVECTOR target, XMVECTOR up, float fov, float ratio, float zNear, float zFar);
	XMMATRIX GetViewProjMatrix();

	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e);

	void OnMouseWheel(MouseWheelEventArgs& e);
	void OnMouseMoved(MouseMotionEventArgs& e);

	void OnMouseButtonPressed(MouseButtonEventArgs& e);
	void OnMouseButtonReleased(MouseButtonEventArgs& e);
};
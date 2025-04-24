#pragma once
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
		float fov = 70.0f, 
		float ratio = 1.0f, 
		float zNear = 0.1f, 
		float zFar = 150.0f);

	void OnUpdate(float deltaTime);
	XMMATRIX GetViewProjMatrix();
	XMMATRIX GetProjMatrix();

	void OnMouseWheel(MouseWheelEventArgs& e);
	void OnMouseMoved(MouseMotionEventArgs& e);
	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e);
	void OnMouseButtonPressed(MouseButtonEventArgs& e);
	void OnMouseButtonReleased(MouseButtonEventArgs& e);
};
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

const static float slowSpeed = 6;
const static float normalSpeed = 12;
const static float fastSpeed = 24;

class Camera
{
private:
	float speed = normalSpeed;

	float sensitivity = 0.005;
	float angle_h = 0.0;
	float angle_v = 0.0;
	float prevX;
	float prevY;
	float dx = 0.0;
	float dy = 0.0;

	float flyRadius = 15.0;

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
		XMVECTOR position = XMVectorSet(0, 0, -5, 1), 
		XMVECTOR target = XMVectorSet(0, 0, 1, 1), 
		XMVECTOR up = XMVectorSet(0, 1, 0, 0), 
		float fov = 70.0f, 
		float ratio = 1.0f, 
		float zNear = 0.1f, 
		float zFar = 300.0f);

	void OnUpdate(float deltaTime);
	XMMATRIX GetViewProjMatrix();

	void OnMouseWheel(MouseWheelEventArgs& e);
	void OnMouseMoved(MouseMotionEventArgs& e);
	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e);
	void OnMouseButtonPressed(MouseButtonEventArgs& e);
	void OnMouseButtonReleased(MouseButtonEventArgs& e);
};
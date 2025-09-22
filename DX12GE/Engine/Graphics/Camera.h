#pragma once

#include "DirectXMath.h"

using namespace DirectX;

class Camera
{
public:
	XMVECTOR Position;
	XMVECTOR Target;  
	XMVECTOR Up;
	
	float Fov;
	float Ratio;
	float ZNear;
	float ZFar;

	void OnLoad(
		XMVECTOR position = XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f),
		XMVECTOR target = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
		float fov = 90.0f, 
		float ratio = 1.0f, 
		float zNear = 0.1f, 
		float zFar = 300.0f);

	XMMATRIX GetViewProjMatrix();
	XMMATRIX GetProjMatrix();	

	void Destroy();
};
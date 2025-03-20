#pragma once
#include "BianObject.h"

class Player
{
private:
	
public:
	float ballRadius = 3;
	float flyRadius = 15.0;

	bool canRotateForward = false;
	bool canRotateBack = false;

	float ballAngle = 0;

	Vector3 Direction;
	float Angle = 0.0;

	BianObject ball;
	BianObject prince;

	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnUpdate(double deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);


	static float GetAngleOfMatrixAndAxis(XMMATRIX rotMat, Vector3 axis);
	static XMMATRIX GetMatrixRotationAxis(Vector3 axis, float angle);

	Vector3 RotationAxis = Vector3(1.0, 0, 0);
	XMMATRIX RotationMatrix = GetMatrixRotationAxis(RotationAxis, 0);
};
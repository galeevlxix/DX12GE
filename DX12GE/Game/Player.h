#pragma once
#include "BianObject.h"


class Player
{
private:
	
public:
	Vector3 Direction;
	BianObject prince;

	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnUpdate(double deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix, bool ShadowMapDrawing = false);

	static float GetAngleOfMatrixAndAxis(XMMATRIX rotMat, Vector3 axis);
	static XMMATRIX GetMatrixRotationAxis(Vector3 axis, float angle);
};
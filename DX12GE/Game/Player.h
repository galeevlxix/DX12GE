#pragma once
#include "../Engine/Graphics/Object3D.h"

class Player
{
private:
	
public:
	Vector3 Direction;
	Object3D prince;

	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnUpdate(double deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);

	static float GetAngleOfMatrixAndAxis(XMMATRIX rotMat, Vector3 axis);
	static XMMATRIX GetMatrixRotationAxis(Vector3 axis, float angle);
};
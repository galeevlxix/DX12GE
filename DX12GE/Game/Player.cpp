#include "Player.h"
#include <iostream>

void Player::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	prince.OnLoad(commandList, "../../DX12GE/Resources/Models/Katamari Objects/prince_katamari_damacy/scene.gltf");
	prince.Transform.SetPosition(-6.7f, 13.3f, 43.0f);
	Direction = Vector3(0.0f, 0.0f, -1.0f);
}

void Player::OnUpdate(double deltaTime)
{
	prince.OnUpdate(deltaTime);
}

void Player::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	prince.OnRender(commandList, viewProjMatrix);
}

XMMATRIX Player::GetMatrixRotationAxis(Vector3 axis, float angle)
{
	float sinValue = sin(angle);
	float cosValue = cos(angle);

	XMMATRIX rotMat = XMMatrixSet
	(
		axis.x * axis.x * (1.0f - cosValue) + cosValue,
		-axis.x * axis.y * (1.0f - cosValue) + axis.z * sinValue,
		axis.x * axis.z * (1.0f - cosValue) + axis.y * sinValue,
		0.0f,

		-axis.y * axis.x * (1.0f - cosValue) - axis.z * sinValue,
		axis.y * axis.y * (1.0f - cosValue) + cosValue,
		axis.y * axis.z * (1.0f - cosValue) + axis.x * sinValue,
		0.0f,

		axis.z * axis.x * (1.0f - cosValue) + axis.y * sinValue,
		-axis.z * axis.y * (1.0f - cosValue) - axis.x * sinValue,
		axis.z * axis.z * (1.0f - cosValue) + cosValue,
		0.0f,

		0.0f, 0.0f, 0.0f, 1.0f
	);

	return rotMat;
}

float Player::GetAngleOfMatrixAndAxis(XMMATRIX rotMat, Vector3 axis)
{
	float val00 = rotMat.r[0].m128_f32[0];
	float val01 = rotMat.r[0].m128_f32[1];
	float val02 = rotMat.r[0].m128_f32[2];
	float val11 = rotMat.r[1].m128_f32[1];
	float val12 = rotMat.r[1].m128_f32[2];
	float val22 = rotMat.r[2].m128_f32[2];

	float cosVal = 0.0f;
	float sinVal = 1.0f;

	if (1.0f - axis.x * axis.x != 0.0f)
	{
		cosVal = (val00 - axis.x * axis.x) / (1.0f - axis.x * axis.x);
	}

	if (1.0f - axis.z * axis.z != 0.0f && (cosVal > 1.0f || cosVal < -1.0f))
	{
		cosVal = (val22 - axis.z * axis.z) / (1.0f - axis.z * axis.z);
	}

	if (1.0f - axis.y * axis.y != 0.0f && (cosVal > 1.0f || cosVal < -1.0f))
	{
		cosVal = (val11 - axis.y * axis.y) / (1.0f - axis.y * axis.y);
	}

	if ((cosVal > 1.0f || cosVal < -1.0f))
	{
		cout << "Ошибка при вычислении угла!" << endl;
	}


	if (axis.x != 0.0f)
	{
		sinVal = (val12 - axis.z * axis.y * (1.0f - cosVal)) / axis.x;
	}
	else if (axis.z != 0.0f)
	{
		sinVal = (val01 + axis.x * axis.y * (1.0f - cosVal)) / axis.z;
	}
	else if (axis.y != 0.0f)
	{
		sinVal = (val02 - axis.z * axis.x * (1.0f - cosVal)) / axis.y;
	}
	else
	{
		cout << "Ошибка при вычислении угла!" << endl;
	}

	float angle = acosf(cosVal);

	if (sinVal < 0.0f)
	{
		angle = PI + (PI - angle);
	}

	if (angle != angle)
	{
		cout << "Ошибка при вычислении угла!" << endl;
	}

	return angle;
}
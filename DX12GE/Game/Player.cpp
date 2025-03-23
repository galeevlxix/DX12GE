#include "Player.h"
#include <iostream>

void Player::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	prince.OnLoad(commandList, "../../DX12GE/Resources/Katamari Objects/prince_katamari_damacy/scene.gltf");
	//prince.SetRotationY(PI);
	prince.Move(0, 2.2f, -3);

	ball.OnLoad(commandList, "../../DX12GE/Resources/Katamari Objects/katamari_ball/core_03.obj");
	ball.Move(0, ballRadius, 0);
	ball.SetScale(100, 100, 100);
}

void Player::OnUpdate(double deltaTime)
{
	Direction.Normalize();
	RotationAxis = Direction;
	RotationAxis = RotationAxis.Cross(Vector3(0, 1, 0));
	RotationAxis.Normalize();
	RotationAxis = RotationAxis * -1;

	//ballAngle = GetAngleOfMatrixAndAxis(RotationMatrix, RotationAxis);

	if (canRotateForward)
	{
		ballAngle += PI * deltaTime;
		if (ballAngle >= 2 * PI) ballAngle -= 2 * PI;
		RotationMatrix = GetMatrixRotationAxis(RotationAxis, ballAngle);
	}	
	if (canRotateBack)
	{
		ballAngle -= PI * deltaTime;
		if (ballAngle < 0) ballAngle += 2 * PI;
		RotationMatrix = GetMatrixRotationAxis(RotationAxis, ballAngle);
	}		

	prince.SetPosition(sin(Angle) * ballRadius + ball.Position.x, prince.Position.y, cos(Angle) * ballRadius + ball.Position.z);

	prince.OnUpdate(deltaTime);
	ball.OnUpdateRotMat(deltaTime, RotationMatrix);
}

void Player::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	prince.OnRender(commandList, viewProjMatrix);  
	ball.OnRender(commandList, viewProjMatrix);
}

XMMATRIX Player::GetMatrixRotationAxis(Vector3 axis, float angle)
{
	float sinValue = sin(angle);
	float cosValue = cos(angle);

	XMMATRIX rotMat = XMMatrixSet
	(
		axis.x * axis.x * (1.0 - cosValue) + cosValue,
		-axis.x * axis.y * (1.0 - cosValue) + axis.z * sinValue,
		axis.x * axis.z * (1.0 - cosValue) + axis.y * sinValue,
		0.0,

		-axis.y * axis.x * (1.0 - cosValue) - axis.z * sinValue,
		axis.y * axis.y * (1.0 - cosValue) + cosValue,
		axis.y * axis.z * (1.0 - cosValue) + axis.x * sinValue,
		0.0,

		axis.z * axis.x * (1.0 - cosValue) + axis.y * sinValue,
		-axis.z * axis.y * (1.0 - cosValue) - axis.x * sinValue,
		axis.z * axis.z * (1.0 - cosValue) + cosValue,
		0.0,

		0.0, 0.0, 0.0, 1.0
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

	float cosVal = 0.0;
	float sinVal = 1.0;

	if (1.0 - axis.x * axis.x != 0.0)
	{
		cosVal = (val00 - axis.x * axis.x) / (1.0 - axis.x * axis.x);
	}

	if (1.0 - axis.z * axis.z != 0.0 && (cosVal > 1.0 || cosVal < -1.0))
	{
		cosVal = (val22 - axis.z * axis.z) / (1.0 - axis.z * axis.z);
	}

	if (1.0 - axis.y * axis.y != 0.0 && (cosVal > 1.0 || cosVal < -1.0))
	{
		cosVal = (val11 - axis.y * axis.y) / (1.0 - axis.y * axis.y);
	}

	if ((cosVal > 1.0 || cosVal < -1.0))
	{
		cout << "Ошибка при вычислении угла!" << endl;
	}


	if (axis.x != 0)
	{
		sinVal = (val12 - axis.z * axis.y * (1.0 - cosVal)) / axis.x;
	}
	else if (axis.z != 0)
	{
		sinVal = (val01 + axis.x * axis.y * (1.0 - cosVal)) / axis.z;
	}
	else if (axis.y != 0)
	{
		sinVal = (val02 - axis.z * axis.x * (1.0 - cosVal)) / axis.y;
	}
	else
	{
		cout << "Ошибка при вычислении угла!" << endl;
	}

	float angle = acosf(cosVal);

	if (sinVal < 0.0)
	{
		angle = PI + (PI - angle);
	}

	if (angle != angle)
	{
		cout << "Ошибка при вычислении угла!" << endl;
	}

	return angle;
}


/*XMMATRIX rotMat = XMMatrixSet
(
	right.x * right.x * (1 - cos(ballAngle)) + cos(ballAngle),
	right.x * right.y * (1 - cos(ballAngle)) - right.z * sin(ballAngle),
	right.x * right.z * (1 - cos(ballAngle)) + right.y * sin(ballAngle),
	0,

	right.y * right.x * (1 - cos(ballAngle)) + right.z * sin(ballAngle),
	right.y * right.y * (1 - cos(ballAngle)) + cos(ballAngle),
	right.y * right.z * (1 - cos(ballAngle)) + right.x * sin(ballAngle),
	0,

	right.z * right.x * (1 - cos(ballAngle)) + right.y * sin(ballAngle),
	right.z * right.y * (1 - cos(ballAngle)) + right.x * sin(ballAngle),
	right.z * right.z * (1 - cos(ballAngle)) + cos(ballAngle),
	0,

	0, 0, 0, 1
);*/


//rotMat = XMMatrixTranspose(rotMat);

	//\cos \theta + (1 - \cos \theta) x ^ 2
	//& (1 - \cos \theta) x y - (\sin \theta) z
	//& (1 - \cos \theta) x z + (\sin \theta) y
	//\\
		//(1 - \cos \theta) y x + (\sin \theta) z
		//& \cos \theta + (1 - \cos \theta) y ^ 2
		//& (1 - \cos \theta) y z - (\sin \theta) x
		//\\
		//(1 - \cos \theta) z x - (\sin \theta) y
		//& (1 - \cos \theta) z y + (\sin \theta) x
		//& \cos \theta + (1 - \cos \theta) z ^ 2

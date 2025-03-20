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
	RotationAxis.Cross(Vector3(0, 1, 0));
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

	prince.SetPosition(
		sin(Angle) * ballRadius + ball.Position.X, 
		prince.Position.Y, 
		cos(Angle) * ballRadius + ball.Position.Z);

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
		axis.X * axis.X * (1.0 - cosValue) + cosValue,
		-axis.X * axis.Y * (1.0 - cosValue) + axis.Z * sinValue,
		axis.X * axis.Z * (1.0 - cosValue) + axis.Y * sinValue,
		0.0,

		-axis.Y * axis.X * (1.0 - cosValue) - axis.Z * sinValue,
		axis.Y * axis.Y * (1.0 - cosValue) + cosValue,
		axis.Y * axis.Z * (1.0 - cosValue) + axis.X * sinValue,
		0.0,

		axis.Z * axis.X * (1.0 - cosValue) + axis.Y * sinValue,
		-axis.Z * axis.Y * (1.0 - cosValue) - axis.X * sinValue,
		axis.Z * axis.Z * (1.0 - cosValue) + cosValue,
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

	if (1.0 - axis.X * axis.X != 0.0)
	{
		cosVal = (val00 - axis.X * axis.X) / (1.0 - axis.X * axis.X);
	}

	if (1.0 - axis.Z * axis.Z != 0.0 && (cosVal > 1.0 || cosVal < -1.0))
	{
		cosVal = (val22 - axis.Z * axis.Z) / (1.0 - axis.Z * axis.Z);
	}

	if (1.0 - axis.Y * axis.Y != 0.0 && (cosVal > 1.0 || cosVal < -1.0))
	{
		cosVal = (val11 - axis.Y * axis.Y) / (1.0 - axis.Y * axis.Y);
	}

	if ((cosVal > 1.0 || cosVal < -1.0))
	{
		cout << "Ошибка при вычислении угла!" << endl;
	}


	if (axis.X != 0)
	{
		sinVal = (val12 - axis.Z * axis.Y * (1.0 - cosVal)) / axis.X;
	}
	else if (axis.Z != 0)
	{
		sinVal = (val01 + axis.X * axis.Y * (1.0 - cosVal)) / axis.Z;
	}
	else if (axis.Y != 0)
	{
		sinVal = (val02 - axis.Z * axis.X * (1.0 - cosVal)) / axis.Y;
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
	right.X * right.X * (1 - cos(ballAngle)) + cos(ballAngle),
	right.X * right.Y * (1 - cos(ballAngle)) - right.Z * sin(ballAngle),
	right.X * right.Z * (1 - cos(ballAngle)) + right.Y * sin(ballAngle),
	0,

	right.Y * right.X * (1 - cos(ballAngle)) + right.Z * sin(ballAngle),
	right.Y * right.Y * (1 - cos(ballAngle)) + cos(ballAngle),
	right.Y * right.Z * (1 - cos(ballAngle)) + right.X * sin(ballAngle),
	0,

	right.Z * right.X * (1 - cos(ballAngle)) + right.Y * sin(ballAngle),
	right.Z * right.Y * (1 - cos(ballAngle)) + right.X * sin(ballAngle),
	right.Z * right.Z * (1 - cos(ballAngle)) + cos(ballAngle),
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

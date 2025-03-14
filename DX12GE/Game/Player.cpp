#include "Player.h"

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

	//XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMVectorSet(Direction.X, Direction.Y, Direction.Z, 1.0), XMVectorSet(0.0, 1.0, 0.0, 1.0)));
	//SimpleMath::Vector3 rightVec(right);

	Vector3 right = Vector3(0, 1, 0);
	Direction.Normalize();
	right.Cross(Direction);
	right.Normalize();

	if (canRotateForward)
	{
		ballAngle += PI * deltaTime;
	}

	XMMATRIX rotMat = XMMatrixSet
	(
		cos(ballAngle) + right.X * right.X * (1 - cos(ballAngle)),	
		right.X * right.Y * (1 - cos(ballAngle)) - right.Z * sin(ballAngle),

	);
	

	

	//SimpleMath::Matrix rotMat;
	//rotMat = SimpleMath::Matrix::CreateFromAxisAngle(rightVec, ballAngle);
	
	// симпл math прикрутить


	prince.SetPosition(
		sin(Angle) * ballRadius + ball.Position.X, 
		prince.Position.Y, 
		cos(Angle) * ballRadius + ball.Position.Z);

	//ball.SetPosition(Vector3(prince.Position.X + ballRadius, ball.Position.Y, prince.Position.Z));

	prince.OnUpdate(deltaTime);
	ball.OnUpdate(deltaTime);
}

void Player::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	prince.OnRender(commandList, viewProjMatrix);
	ball.OnRender(commandList, viewProjMatrix);
}


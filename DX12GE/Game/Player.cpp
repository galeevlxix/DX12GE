#include "Player.h"

void Player::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	prince.OnLoad(commandList, "../../DX12GE/Resources/Katamari Objects/prince_katamari_damacy/scene.gltf");
	//prince.SetRotationY(PI);
	prince.Move(0, 2.2f, -3);

	ball.OnLoad(commandList, "../../DX12GE/Resources/Katamari Objects/katamari_ball/core_01.obj");
	ball.Move(0, ballRadius, 0);
	ball.SetScale(100, 100, 100);
}

void Player::OnUpdate(double deltaTime)
{
	ball.SetRotation(ball.Rotation.X, Angle + PI, ball.Rotation.Z);

	static Vector3 rotSpeed(PI, 0, 0);

	if (canRotateForward)
	{
		ball.Rotate(rotSpeed * deltaTime);
	} 
	
	if (canRotateBack)
	{
		ball.Rotate(rotSpeed * -deltaTime);
	}

	prince.SetPosition(
		sin(Angle) * ballRadius + ball.Position.X, 
		prince.Position.Y, 
		cos(Angle) * ballRadius + ball.Position.Z);
	prince.SetRotation(prince.Rotation.X, Angle + PI, prince.Rotation.Z);

	prince.OnUpdate(deltaTime);
	ball.OnUpdate(deltaTime);
}

void Player::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	prince.OnRender(commandList, viewProjMatrix);
	ball.OnRender(commandList, viewProjMatrix);
}

Vector3 Player::GetPosition()
{
	return ball.Position;
}

void Player::Move(Vector3 MoveVector)
{
	ball.Move(MoveVector);
}


#include "SolarSystem.h"
#include <math.h>

void SolarSystem::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	Sun.CreateSphereGeometry(32, 32);
	Sun.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(20, 20, 20), Vector3(0, 0, 0));

	Mercury.CreateSphereGeometry(16, 8);
	Mercury.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(8, 8, 8), Vector3(0, 0, 0));

	Satellite1.CreateCubeGeometry();
	Satellite1.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0, 0, 0));

	Satellite2.CreateSphereGeometry(16, 16);
	Satellite2.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(2, 2, 2), Vector3(1, 0, 0));

	Venus.CreateCubeGeometry();
	Venus.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(5, 5, 5), Vector3(0, 0, 0));

	Satellite3.CreateSphereGeometry(8, 8);
	Satellite3.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(2, 2, 2), Vector3(0.5, 0.5, 1));

	Earth.CreateSphereGeometry(24, 8);
	Earth.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(6, 6, 6), Vector3(0, 0, 0));

	Moon.CreateSphereGeometry(16, 16);
	Moon.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0.5, 0.5, 0.5));

	Mars.CreateSphereGeometry(16, 8);
	Mars.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(7, 7, 7), Vector3(0, 0, 0));

	Satellite4.CreateCubeGeometry();
	Satellite4.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0, 0, 0));

	Satellite5.CreateSphereGeometry(8, 8);
	Satellite5.OnLoad(
		commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(1, 1, 1), Vector3(0, 0, 0));
}

void SolarSystem::OnUpdate(double deltaTime)
{
	static float speed = 0;
	speed += deltaTime / 2;

	static float sunRotSpeed = PI / 8;
	static float planetsRotSpeed = sunRotSpeed * 4;
	static float satellitesRotSpeed = planetsRotSpeed * 4;

	Sun.SetPosition(0, sin(speed) * 5, 0);
	Sun.Rotate(Vector3(0, sunRotSpeed * deltaTime, 0));

	Mercury.SetPosition(
		Sun.GetPosition() + Vector3(sin(speed * 2), 0, cos(speed * 2)) * 40);
	Mercury.Rotate(Vector3(0, planetsRotSpeed * deltaTime, 0));
	Venus.SetPosition(
		Sun.GetPosition() + Vector3(sin(speed * 1.2), 0, cos(speed * 1.2)) * 55);
	Venus.Rotate(Vector3(0, planetsRotSpeed * deltaTime, 0));
	Earth.SetPosition(
		Sun.GetPosition() + Vector3(sin(speed), 0, cos(speed)) * 70);
	Earth.Rotate(Vector3(0, planetsRotSpeed * deltaTime, 0));
	Mars.SetPosition(
		Sun.GetPosition() + Vector3(sin(speed * 0.8), 0, cos(speed * 0.8)) * 100);
	Mars.Rotate(Vector3(0, planetsRotSpeed * deltaTime, 0));

	Moon.SetPosition(
		Earth.GetPosition() + Vector3(sin(speed * 5), 0, cos(speed * 5)) * 10);
	Moon.Rotate(Vector3(0, satellitesRotSpeed * deltaTime, 0));
	Satellite1.SetPosition(
		Mercury.GetPosition() + Vector3(sin(speed * 10), 0, cos(speed * 10)) * 10);
	Satellite1.Rotate(Vector3(0, satellitesRotSpeed * deltaTime, 0));
	Satellite2.SetPosition(
		Mercury.GetPosition() + Vector3(sin(speed * 5), 0, cos(speed * 5)) * 15);
	Satellite2.Rotate(Vector3(0, satellitesRotSpeed * deltaTime, 0));
	Satellite3.SetPosition(
		Venus.GetPosition() + Vector3(sin(speed * 10), 0, cos(speed * 10)) * 10);
	Satellite3.Rotate(Vector3(0, satellitesRotSpeed * deltaTime, 0));
	Satellite4.SetPosition(
		Mars.GetPosition() + Vector3(sin(speed * 5), 0, cos(speed * 5)) * 15);
	Satellite4.Rotate(Vector3(0, satellitesRotSpeed * deltaTime, 0));
	Satellite5.SetPosition(
		Mars.GetPosition() + Vector3(sin(speed * 10), 0, cos(speed * 10)) * 25);
	Satellite5.Rotate(Vector3(0, satellitesRotSpeed * deltaTime, 0));

	Sun.OnUpdate(deltaTime);
	Mercury.OnUpdate(deltaTime);
	Venus.OnUpdate(deltaTime);
	Earth.OnUpdate(deltaTime);
	Mars.OnUpdate(deltaTime);

	Moon.OnUpdate(deltaTime);
	Satellite1.OnUpdate(deltaTime);
	Satellite2.OnUpdate(deltaTime);
	Satellite3.OnUpdate(deltaTime);
	Satellite4.OnUpdate(deltaTime);
	Satellite5.OnUpdate(deltaTime);
}

void SolarSystem::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	Sun.OnRender(commandList, viewProjMatrix);
	Mercury.OnRender(commandList, viewProjMatrix);
	Venus.OnRender(commandList, viewProjMatrix);
	Earth.OnRender(commandList, viewProjMatrix);
	Mars.OnRender(commandList, viewProjMatrix);

	Moon.OnRender(commandList, viewProjMatrix);
	Satellite1.OnRender(commandList, viewProjMatrix);
	Satellite2.OnRender(commandList, viewProjMatrix);
	Satellite3.OnRender(commandList, viewProjMatrix);
	Satellite4.OnRender(commandList, viewProjMatrix);
	Satellite5.OnRender(commandList, viewProjMatrix);
}

Vector3 SolarSystem::GetPlanetPosition()
{
	return Sun.GetPosition();
}

float SolarSystem::GetNewFov(Vector3 CameraPos, float ratio)
{
	float distance = (GetPlanetPosition() - CameraPos).Length();
	float tg = Sun.radius / distance;
	float angle = 2 * XMConvertToDegrees(atan(tg));

	return angle;
}

//катамари: текстуры и ассимп
//освещение: поинт и директ
//тени: камкадные
//деферед рендеринг
//частицы на гпу

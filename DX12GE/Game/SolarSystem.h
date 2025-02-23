#pragma once
#include "BaseObject.h"
#include <string>
#include <map>

using namespace std;

class SolarSystem
{
public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnUpdate(double deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);

	Vector3 GetPlanetPosition();
private:
	BaseObject Sun;

	BaseObject Mercury;
	BaseObject Venus;
	BaseObject Earth;
	BaseObject Mars;

	BaseObject Moon;
	BaseObject Satellite1;
	BaseObject Satellite2;
	BaseObject Satellite3;
	BaseObject Satellite4;
	BaseObject Satellite5;
};
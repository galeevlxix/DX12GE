#pragma once
#include "BaseObject.h"
#include <iostream>
#include <map>
using namespace std;

class GlobalScene
{
private:
	map<string, BaseObject> objects;
public:
	GlobalScene();
	~GlobalScene();

	/*void AddCube(string name, ComPtr<ID3D12GraphicsCommandList2>commandList, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 Color);
	void AddSphere(string name, ComPtr<ID3D12GraphicsCommandList2>commandList, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 Color);
	void Remove(string name);

	BaseObject& operator[](string name);
	int GetSize();

	void OnUpdate(double deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);*/
};
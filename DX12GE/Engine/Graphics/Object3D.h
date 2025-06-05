#pragma once

#include "Mesh3D.h"
#include <vector>
#include "Material.h"

using namespace std;

class Object3D
{
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath);
    void OnUpdate(double deltaTime);
    void OnUpdateRotMat(double deltaTime, XMMATRIX rotMat);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);

    void SetPosition(float x, float y, float z);
    void SetPosition(Vector3 PositionVector);
    void Move(float dx, float dy, float dz);
    void Move(Vector3 MoveVector);
    void SetRotation(float x, float y, float z);
    void SetRotation(Vector3 RotationVector);
    void SetRotationX(float value);
    void SetRotationY(float value);
    void SetRotationZ(float value);
    void Rotate(Vector3 RotateVector);
    void SetScale(float x, float y, float z);
    void SetScale(Vector3 ScaleVector);
    void Expand(float ExpandValue);

    Vector3 Position;
    Vector3 Rotation;
private:
	vector<Mesh3D*> m_Meshes;
    vector<Material*> m_Materials;
    vector<int> m_MaterialIndices;
};
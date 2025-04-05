#include "BianObject.h"
#include <string>
#include "../Engine/Application.h"
#include "../Engine/VertexStructures.h"
#include "../Engine/AssimpModelLoader.h"

void BianObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath)
{
    AssimpModelLoader modelLoader;

    float yOffset;
    if (modelLoader.LoadModelData(commandList, filePath, &m_Meshes, &m_Materials, &m_MaterialIndices, &yOffset))
    {
        Move(0, -yOffset, 0);
        Position = Vector3(0, 0, 0);
    }
}

void BianObject::OnUpdate(double deltaTime )
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].OnUpdate();
    }
}

void BianObject::OnUpdateRotMat(double deltaTime, XMMATRIX rotMat)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].OnUpdateByRotationMatrix(deltaTime, rotMat);
    }
}

void BianObject::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix, bool ShadowMapDrawing)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        if (m_Materials[m_MaterialIndices[i]].CanDrawIt())
        {
            if (!ShadowMapDrawing) m_Materials[m_MaterialIndices[i]].Render(commandList);
            m_Meshes[i].OnRender(commandList, viewProjMatrix, ShadowMapDrawing);
        }        
    }
}

void BianObject::SetPosition(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetPosition(x, y, z);
    }
    Position = Vector3(x, y, z);
}

void BianObject::SetPosition(Vector3 PositionVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetPosition(PositionVector);
    }

    Position = PositionVector;
}

void BianObject::Move(float dx, float dy, float dz)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Move(dx, dy, dz);
    }

    Position = Position + Vector3(dx, dy, dz);
}

void BianObject::Move(Vector3 MoveVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Move(MoveVector);
    }
    Position = Position + MoveVector;
}

void BianObject::SetRotation(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotation(x, y, z);
    }
    Rotation = Vector3(x, y, z);
}

void BianObject::SetRotation(Vector3 RotationVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotation(RotationVector);
    }
    Rotation = RotationVector;
}

void BianObject::SetRotationX(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotationX(value);
    }
    Rotation = Vector3(value, Rotation.y, Rotation.z);
}

void BianObject::SetRotationY(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotationY(value);
    }
    Rotation = Vector3(Rotation.x, value, Rotation.z);
}

void BianObject::SetRotationZ(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotationZ(value);
    }
    Rotation = Vector3(Rotation.x, Rotation.y, value);
}

void BianObject::Rotate(Vector3 RotateVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Rotate(RotateVector);
    }
    Rotation = Rotation + RotateVector;
}

void BianObject::SetScale(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetScale(x, y, z);
    }
}

void BianObject::SetScale(Vector3 ScaleVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetScale(ScaleVector);
    }
}

void BianObject::Expand(float ExpandValue)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Expand(ExpandValue);
    }
}

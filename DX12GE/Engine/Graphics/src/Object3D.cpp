#include "../Object3D.h"
#include <string>
#include "../../Base/Application.h"
#include "../VertexStructures.h"
#include "../AssimpModelLoader.h"

void Object3D::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath)
{
    AssimpModelLoader modelLoader;

    float yOffset;
    if (modelLoader.LoadModelData(commandList, filePath, m_Meshes, m_Materials, m_MaterialIndices, yOffset))
    {
        Move(0, -yOffset, 0);
        Position = Vector3(0, 0, 0);
    }
}

void Object3D::OnUpdate(double deltaTime )
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->OnUpdate();
    }
}

void Object3D::OnUpdateRotMat(double deltaTime, XMMATRIX rotMat)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->OnUpdateByRotationMatrix(deltaTime, rotMat);
    }
}

void Object3D::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        if (m_Materials[m_MaterialIndices[i]]->CanDrawIt())
        {
            if (!Mesh3D::GetShadowPass() && !Mesh3D::GetLightPass()) 
                m_Materials[m_MaterialIndices[i]]->Render(commandList);
            m_Meshes[i]->OnRender(commandList, viewProjMatrix);
        }        
    }
}

void Object3D::SetPosition(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetPosition(x, y, z);
    }
    Position = Vector3(x, y, z);
}

void Object3D::SetPosition(Vector3 PositionVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetPosition(PositionVector);
    }

    Position = PositionVector;
}

void Object3D::Move(float dx, float dy, float dz)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->Move(dx, dy, dz);
    }

    Position = Position + Vector3(dx, dy, dz);
}

void Object3D::Move(Vector3 MoveVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->Move(MoveVector);
    }
    Position = Position + MoveVector;
}

void Object3D::SetRotation(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetRotation(x, y, z);
    }
    Rotation = Vector3(x, y, z);
}

void Object3D::SetRotation(Vector3 RotationVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetRotation(RotationVector);
    }
    Rotation = RotationVector;
}

void Object3D::SetRotationX(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetRotationX(value);
    }
    Rotation = Vector3(value, Rotation.y, Rotation.z);
}

void Object3D::SetRotationY(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetRotationY(value);
    }
    Rotation = Vector3(Rotation.x, value, Rotation.z);
}

void Object3D::SetRotationZ(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetRotationZ(value);
    }
    Rotation = Vector3(Rotation.x, Rotation.y, value);
}

void Object3D::Rotate(Vector3 RotateVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->Rotate(RotateVector);
    }
    Rotation = Rotation + RotateVector;
}

void Object3D::SetScale(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetScale(x, y, z);
    }
}

void Object3D::SetScale(Vector3 ScaleVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->SetScale(ScaleVector);
    }
}

void Object3D::Expand(float ExpandValue)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i]->Expand(ExpandValue);
    }
}

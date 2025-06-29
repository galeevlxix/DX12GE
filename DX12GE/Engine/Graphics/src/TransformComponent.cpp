#pragma once
#include "../TransformComponent.h"

DirectX::XMMATRIX TransformComponent::GetWorldMatrix()
{
    return 
        DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
        DirectX::XMMatrixRotationX(m_Rotation.x) *
        DirectX::XMMatrixRotationY(m_Rotation.y) *
        DirectX::XMMatrixRotationZ(m_Rotation.z) *
        DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
}

void TransformComponent::SetDefault(float yOffset)
{
    m_Position = Vector3(0, -yOffset, 0);
    m_Rotation = Vector3(0, 0, 0);
    m_Scale = Vector3(1, 1, 1);
}

void TransformComponent::SetPosition(float x, float y, float z)
{
    m_Position = Vector3(x, y, z);
}

void TransformComponent::SetPosition(Vector3 PositionVector)
{
    m_Position = PositionVector;
}

void TransformComponent::Move(float dx, float dy, float dz)
{
    m_Position += Vector3(dx, dy, dz);
}

void TransformComponent::Move(Vector3 MoveVector)
{
    m_Position += MoveVector;
}

void TransformComponent::SetRotation(float x, float y, float z)
{
    m_Rotation = Vector3(x, y, z);
}

void TransformComponent::SetRotation(Vector3 RotationVector)
{
    m_Rotation = RotationVector;
}

void TransformComponent::SetRotationX(float value)
{
    m_Rotation.x = value;
}

void TransformComponent::SetRotationY(float value)
{
    m_Rotation.y = value;
}

void TransformComponent::SetRotationZ(float value)
{
    m_Rotation.z = value;
}

void TransformComponent::Rotate(Vector3 RotateVector)
{
    m_Rotation += RotateVector;
}

void TransformComponent::SetScale(float x, float y, float z)
{
    m_Scale = Vector3(x, y, z);
}

void TransformComponent::SetScale(float value)
{
    m_Scale = Vector3(value, value, value);
}

void TransformComponent::SetScale(Vector3 ScaleVector)
{
    m_Scale = ScaleVector;
}

void TransformComponent::Expand(float ExpandValue)
{
    m_Scale *= ExpandValue;
}

Vector3 TransformComponent::GetPosition()
{
    return m_Position;
}

Vector3 TransformComponent::GetRotation()
{
    return m_Rotation;
}

Vector3 TransformComponent::GetScale()
{
    return m_Scale;
}

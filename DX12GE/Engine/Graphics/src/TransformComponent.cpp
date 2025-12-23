#pragma once
#include "../TransformComponent.h"
#include <algorithm>

const static float PI = 3.1415926536f;

const DirectX::XMMATRIX& TransformComponent::GetLocalMatrix()
{
    if (m_CacheIsDirty)
    {
        m_LocalMatrixCache = 
            DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
            DirectX::XMMatrixRotationX(m_Rotation.x) *
            DirectX::XMMatrixRotationY(m_Rotation.y) *
            DirectX::XMMatrixRotationZ(m_Rotation.z) *
            DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    }
    return m_LocalMatrixCache;
}

const DirectX::XMMATRIX TransformComponent::GetLocalScaleMatrix()
{
    return DirectX::XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
}

const DirectX::XMMATRIX TransformComponent::GetLocalRotationMatrix()
{
    return
        DirectX::XMMatrixRotationX(m_Rotation.x) *
        DirectX::XMMatrixRotationY(m_Rotation.y) *
        DirectX::XMMatrixRotationZ(m_Rotation.z);
}

void TransformComponent::SetDefault(float yOffset)
{
    SetPosition(Vector3(0.0f, -yOffset, 0.0f));
    SetRotation(Vector3(0.0f, 0.0f, 0.0f));
    SetScale(Vector3(1.0f, 1.0f, 1.0f));
}

float TransformComponent::ToDegrees(float radians)
{
    return radians * 180.0f / PI;
}

float TransformComponent::ToRadians(float degrees)
{
    return degrees * PI / 180.0f;
}

// POSITION

void TransformComponent::SetPosition(Vector3 PositionVector)
{
    m_Position = PositionVector;
    m_CacheIsDirty = true;
}

void TransformComponent::SetPosition(float x, float y, float z)
{
    SetPosition(Vector3(x, y, z));
}

void TransformComponent::Move(float dx, float dy, float dz)
{
    SetPosition(m_Position + Vector3(dx, dy, dz));
}

void TransformComponent::Move(Vector3 MoveVector)
{
    SetPosition(m_Position + MoveVector);
}

// ROTATION

void TransformComponent::SetRotation(Vector3 RotationVector)
{
    m_Rotation = RotationVector;
    m_CacheIsDirty = true;
}

void TransformComponent::SetRotation(float x, float y, float z)
{
    SetRotation(Vector3(x, y, z));
}

void TransformComponent::SetRotationDegrees(Vector3 RotationVector)
{
    SetRotation(ToRadians(RotationVector.x), ToRadians(RotationVector.y), ToRadians(RotationVector.z));
}

void TransformComponent::SetRotationX(float value)
{
    SetRotation(value, m_Rotation.y, m_Rotation.z);
}

void TransformComponent::SetRotationY(float value)
{
    SetRotation(m_Rotation.x, value, m_Rotation.z);
}

void TransformComponent::SetRotationZ(float value)
{
    SetRotation(m_Rotation.x, m_Rotation.y, value);
}

void TransformComponent::Rotate(Vector3 RotateVector)
{
    SetRotation(m_Rotation + RotateVector);
}

void TransformComponent::RotateDegrees(Vector3 RotateVector)
{
    SetRotation(m_Rotation + Vector3(ToRadians(RotateVector.x), ToRadians(RotateVector.y), ToRadians(RotateVector.z)));
}

void TransformComponent::Rotate(float dx, float dy, float dz)
{
    SetRotation(m_Rotation + Vector3(dx, dy, dz));
}

void TransformComponent::LocalLookAt(const Vector3& localTarget)
{
    Vector3 dir = localTarget - m_Position;
    dir.Normalize();

    float yaw = atan2(dir.x, dir.z);
    float pitch = asin(-dir.y);

    pitch = std::clamp(pitch, -PI * 0.5f + 0.001f, PI * 0.5f - 0.001f);

    m_Rotation = Vector3(pitch, yaw, 0.0f);
}

void TransformComponent::RotateToLocalDirection(const Vector3& direction)
{
    float yaw = atan2(direction.x, direction.z);
    float pitch = asin(-direction.y);

    pitch = std::clamp(pitch, -PI * 0.5f + 0.001f, PI * 0.5f - 0.001f);

    m_Rotation = Vector3(pitch, yaw, 0.0f);
}

// SCALE

void TransformComponent::SetScale(Vector3 ScaleVector)
{
    m_Scale = ScaleVector;
    m_CacheIsDirty = true;
}

void TransformComponent::SetScale(float x, float y, float z)
{
    SetScale(Vector3(x, y, z));
}

void TransformComponent::SetScale(float value)
{
    SetScale(Vector3(value, value, value));
}

void TransformComponent::Expand(float ExpandValue)
{
    SetScale(m_Scale * ExpandValue);
}

void TransformComponent::Expand(float dx, float dy, float dz)
{
    SetScale(m_Scale * Vector3(dx, dy, dz));
}

// GET

const Vector3& TransformComponent::GetPosition()
{
    return m_Position;
}

const Vector3& TransformComponent::GetRotation()
{
    return m_Rotation;
}

const Vector3 TransformComponent::GetRotationDegrees()
{
    return Vector3(ToDegrees(m_Rotation.x), ToDegrees(m_Rotation.y), ToDegrees(m_Rotation.z));
}

const Vector3& TransformComponent::GetScale()
{
    return m_Scale;
}

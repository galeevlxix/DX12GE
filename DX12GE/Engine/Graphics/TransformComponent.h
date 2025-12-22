#pragma once
#include "../Base/SimpleMath.h"
using namespace DirectX::SimpleMath;

class TransformComponent
{
    DirectX::XMMATRIX m_LocalMatrixCache;
    bool m_CacheIsDirty = true;

    Vector3 m_Position;
    Vector3 m_Rotation;
    Vector3 m_Scale;

    friend void SetTransformCacheStatus(TransformComponent& transform, bool cacheIsDirty);

    static float ToDegrees(float radians);
    static float ToRadians(float degrees);

public:
    const DirectX::XMMATRIX& GetLocalMatrix();

    void SetDefault(float yOffset = 0.0f);

    void SetPosition(Vector3 PositionVector);
    void SetPosition(float x, float y, float z);
    void Move(Vector3 MoveVector);
    void Move(float dx, float dy, float dz);

    void SetRotation(Vector3 RotationVector);
    void SetRotationDegrees(Vector3 RotationVector);
    void SetRotation(float x, float y, float z);
    void SetRotationX(float value);
    void SetRotationY(float value);
    void SetRotationZ(float value);
    void Rotate(Vector3 RotateVector);
    void RotateDegrees(Vector3 RotateVector);
    void Rotate(float dx, float dy, float dz);

    void LocalLookAt(const Vector3& localTarget);

    void SetScale(Vector3 ScaleVector);
    void SetScale(float x, float y, float z);
    void SetScale(float value);
    void Expand(float ExpandValue);
    void Expand(float dx, float dy, float dz);

    const Vector3& GetPosition();
    const Vector3& GetRotation();
    const Vector3 GetRotationDegrees();
    const Vector3& GetScale();

    bool IsCacheDirty() { return m_CacheIsDirty; }
};
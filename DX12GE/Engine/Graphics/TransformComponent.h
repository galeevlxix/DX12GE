#pragma once
#include "../Base/SimpleMath.h"
using namespace DirectX::SimpleMath;

class TransformComponent
{
public:
    DirectX::XMMATRIX GetWorldMatrix();
    void SetDefault(float yOffset = 0);

    
    static float ToDegrees(float radians);
    static float ToRadians(float degrees);

    void SetPosition(float x, float y, float z);
    void SetPosition(Vector3 PositionVector);
    void Move(float dx, float dy, float dz);
    void Move(Vector3 MoveVector);
    void SetRotation(float x, float y, float z);
    void SetRotation(Vector3 RotationVector);
    void SetRotationDegrees(Vector3 RotationVector);
    void SetRotationX(float value);
    void SetRotationY(float value);
    void SetRotationZ(float value);
    void Rotate(Vector3 RotateVector);
    void RotateDegrees(Vector3 RotateVector);
    void Rotate(float dx, float dy, float dz);
    void SetScale(float x, float y, float z);
    void SetScale(float value);
    void SetScale(Vector3 ScaleVector);
    void Expand(float ExpandValue);
    void Expand(float dx, float dy, float dz);

    Vector3 GetPosition();
    Vector3 GetRotation();
    Vector3 GetScale();

private:
    Vector3 m_Position;
    Vector3 m_Rotation;
    Vector3 m_Scale;    
};
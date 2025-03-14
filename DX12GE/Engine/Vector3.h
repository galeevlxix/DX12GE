#pragma once

#include "DX12LibPCH.h"
using namespace DirectX;

class Vector3
{
public:
    float X;
    float Y;
    float Z;

    Vector3() : X(0), Y(0), Z(0) {};
    Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {};

    static Vector3 Zero();
    static Vector3 One();

    void Set(float x, float y, float z);
    void Increase(float dx, float dy, float dz);
    void Normalize();
    float Length();
    
    void Cross(Vector3 rVec);

    XMFLOAT3 ToXM();
    XMVECTOR ToXMVector();

    Vector3 operator*(float value);
    Vector3 operator+(float value);
    Vector3 operator-(float value);
    Vector3 operator+(Vector3 value);
    Vector3 operator-(Vector3 value);
};

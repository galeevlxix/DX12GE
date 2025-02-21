#include "Vector3.h"
#include "math.h"

Vector3 Vector3::Zero()
{
    return Vector3(0, 0, 0);
}

Vector3 Vector3::One()
{
    return Vector3(1, 1, 1);
}

void Vector3::Set(float x, float y, float z)
{
    X = x;
    Y = y;
    Z = z;
}

void Vector3::Increase(float dx, float dy, float dz)
{
    X += dx;
    Y += dy;
    Z += dz;
}

void Vector3::Normalize()
{
    float length = X * X + Y * Y + Z * Z;
    if (length != 0)
    {
        Set(X / length, Y / length, Z / length);
    }
}

float Vector3::Length()
{
    return sqrtf(X * X + Y * Y + Z * Z);
}

XMFLOAT3 Vector3::ToXM()
{
    return XMFLOAT3(X, Y, Z);
}

Vector3 Vector3::operator*(float value)
{
    return Vector3(X * value, Y * value, Z * value);
}

Vector3 Vector3::operator+(float value)
{
    return Vector3(X + value, Y + value, Z + value);
}

Vector3 Vector3::operator+(Vector3 value)
{
    return Vector3(X + value.X, Y + value.Y, Z + value.Z);
}

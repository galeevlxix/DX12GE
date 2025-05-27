#pragma once
#include "DirectXMath.h"

using namespace DirectX;

struct VertexStruct
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT2 TexCoord;
    XMFLOAT3 Tangent;
    XMFLOAT3 Bitangent;
};

struct VertexPositionColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

struct VertexPositionTextCoord
{
    XMFLOAT3 Position;
    XMFLOAT2 TextCoord;
};

struct VertexParticle
{
    XMFLOAT3 Position;
    XMFLOAT3 Speed;

    VertexParticle()
    {
        Position = XMFLOAT3(0, 0, 0);
        Speed = XMFLOAT3(0, 0, 0);
    }

    VertexParticle(XMFLOAT3 pos, XMFLOAT3 velocity)
    {
        Position = pos;
        Speed = velocity;
    }
};

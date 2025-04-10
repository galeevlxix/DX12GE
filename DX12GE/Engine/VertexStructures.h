#pragma once
#include "DirectXMath.h"

using namespace DirectX;

struct VertexStruct
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT2 TexCoord;
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

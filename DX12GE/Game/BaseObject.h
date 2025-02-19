#pragma once
#include "../Engine/CommandQueue.h"

using namespace DirectX;

// Vertex data for a colored cube
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

class Vector3
{
public:
    float X;
    float Y;
    float Z;

    void Set(float x, float y, float z);
    void Increase(float dx, float dy, float dz);
};

class BaseObject
{
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, float x, float y, float z);
    void OnUpdate(double totalTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);

    void SetPosition(float x, float y, float z);
    void Move(float dx, float dy, float dz);
    void SetRotation(float x, float y, float z);
    void SetRotationX(float value);
    void SetRotationY(float value);
    void SetRotationZ(float value);
    void SetScale(float x, float y, float z);

private:
    XMMATRIX m_ModelMatrix;
    /*XMMATRIX m_PositionMatrix;
    XMMATRIX m_RotationXMatrix;
    XMMATRIX m_RotationYMatrix;
    XMMATRIX m_RotationZMatrix;
    XMMATRIX m_ScaleMatrix; */

    Vector3 m_Position;
    Vector3 m_Rotation;
    Vector3 m_Scale;

    // Vertex buffer for the cube.
    ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    ComPtr<ID3D12Resource> intermediateVertexBuffer;

    // Index buffer for the cube.
    ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
    ComPtr<ID3D12Resource> intermediateIndexBuffer;

    VertexPosColor m_Vertices[8] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },  // 0
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },  // 1
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },   // 2
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },   // 3
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },  // 4
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },  // 5
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },   // 6
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }    // 7
    };

    WORD m_Indicies[36] =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

    void TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
    void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
};


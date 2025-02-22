#pragma once
#include "../Engine/CommandQueue.h"
#include "../Engine/Vector3.h"

#include <vector>

using namespace std;

using namespace DirectX;

// Vertex data for a colored cube
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

class BaseObject
{
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 Color);
    void OnUpdate(double deltaTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);

    void SetPosition(float x, float y, float z);
    void SetPosition(Vector3 PositionVector);
    void Move(float dx, float dy, float dz);
    void Move(Vector3 MoveVector);
    void SetRotation(float x, float y, float z);
    void SetRotation(Vector3 RotationVector);
    void SetRotationX(float value);
    void SetRotationY(float value);
    void SetRotationZ(float value);
    void SetScale(float x, float y, float z);
    void SetScale(Vector3 ScaleVector);

    Vector3 GetPosition();
    Vector3 GetRotation();
    Vector3 GetScale();

    void CreateSphereGeometry();
    void CreateCubeGeometry();
private:
    XMMATRIX m_ModelMatrix;

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

    vector<VertexPosColor> m_Vertices;
    vector<WORD> m_Indices;
    int indiciesCount;

    void TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
    void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
};


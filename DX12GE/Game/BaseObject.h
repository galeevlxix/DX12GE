#pragma once
#include "../Engine/CommandQueue.h"                                                                          
#include "../Engine/SimpleMath.h"
#include <vector>
#include <string>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define PI 3.1415926535f

class BaseObject
{
public:
    template<typename T>
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const vector<T> vertices, const vector<WORD> indices);
    void Release();

    void OnUpdate();
    void OnUpdateByRotationMatrix(double deltaTime, XMMATRIX rotMat);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
    void OnRenderLineList(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
    void OnRenderPointList(ComPtr<ID3D12GraphicsCommandList2> commandList);

    static void SetSMMatrices(Matrix m[]);
    static void DebugMatrices();
    static void SetShadowPass(bool isShadowPass);
    static bool GetShadowPass();
    static void SetGeometryPass(bool isGeometryPass);
    static bool GetGeometryPass();
    static void SetLightPass(bool isLightPass);
    static bool GetLightPass();

    void SetDefaultState();
    void SetPosition(float x, float y, float z);
    void SetPosition(Vector3 PositionVector);
    void Move(float dx, float dy, float dz);
    void Move(Vector3 MoveVector);
    void SetRotation(float x, float y, float z);
    void SetRotation(Vector3 RotationVector);
    void SetRotationX(float value);
    void SetRotationY(float value);
    void SetRotationZ(float value);
    void Rotate(Vector3 RotateVector);
    void SetScale(float x, float y, float z);
    void SetScale(Vector3 ScaleVector);
    void Expand(float value);

    Vector3 GetPosition();
    Vector3 GetRotation();
    Vector3 GetScale();

private:
    XMMATRIX m_WorldMatrix;
    Vector3 m_Position;
    Vector3 m_Rotation;
    Vector3 m_Scale;

    // Vertex buffer for the cube.
    ComPtr<ID3D12Resource> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    ComPtr<ID3D12Resource> IntermediateVertexBufferResource;

    // Index buffer for the cube.
    ComPtr<ID3D12Resource> m_IndexBuffer;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
    ComPtr<ID3D12Resource> IntermediateIndexBufferResource;

    UINT indiciesCount;

    bool Initialized = false;

    void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

public:
    bool IsInitialized() { return Initialized; }
};

template<typename T>
void BaseObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const vector<T> vertices, const vector<WORD> indices)
{
    indiciesCount = static_cast<UINT>(indices.size());

    // Загрузить данные вершинного буфера
    UpdateBufferResource(commandList, &m_VertexBuffer, &IntermediateVertexBufferResource, vertices.size(), sizeof(T), vertices.data());

    // Создать представление буфера вершин
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(T));
    m_VertexBufferView.StrideInBytes = sizeof(T);

    // Загрузить данные индексного буфера
    UpdateBufferResource(commandList, &m_IndexBuffer, &IntermediateIndexBufferResource, indices.size(), sizeof(WORD), indices.data());

    // Создать представление индексного буфера
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(WORD));

    SetDefaultState();

    Initialized = true;
}
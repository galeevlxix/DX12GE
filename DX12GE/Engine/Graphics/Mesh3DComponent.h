#pragma once

#include "../Base/SimpleMath.h"
#include "ShaderResources.h"

#include "MaterialEntity.h"
#include <vector>
#include <string>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

class Mesh3DComponent
{
public:
    template<typename T>
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const vector<T> vertices, const vector<WORD> indices, bool urgently = false);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D_PRIMITIVE_TOPOLOGY primitiveType);
    void Destroy();  

private:

    ComPtr<ID3D12Resource> m_VertexBuffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    ComPtr<ID3D12Resource> m_IntermediateVertexBufferResource = nullptr;

    ComPtr<ID3D12Resource> m_IndexBuffer = nullptr;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
    ComPtr<ID3D12Resource> m_IntermediateIndexBufferResource = nullptr;

    UINT m_IndiciesCount;
    UINT m_VerticesCount;
    bool m_Initialized = false;

    void UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>& pDestinationResource, ComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

public:

    MaterialEntity* m_Material;

    bool IsInitialized() { return m_Initialized; }
};

template<typename T>
void Mesh3DComponent::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const vector<T> vertices, const vector<WORD> indices, bool urgently)
{
    m_IndiciesCount = static_cast<UINT>(indices.size());
    m_VerticesCount = static_cast<UINT>(vertices.size());

    UpdateBufferResource(commandList, m_VertexBuffer, m_IntermediateVertexBufferResource, vertices.size(), sizeof(T), vertices.data());
    m_VertexBuffer->SetName(L"Mesh3DVertexBufferResource");
    m_IntermediateVertexBufferResource->SetName(L"Mesh3DIntermediateVertexBufferResource");

    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(T));
    m_VertexBufferView.StrideInBytes = sizeof(T);

    if (urgently)
        TransitionResource(commandList, m_VertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    UpdateBufferResource(commandList, m_IndexBuffer, m_IntermediateIndexBufferResource, indices.size(), sizeof(WORD), indices.data());
    m_IndexBuffer->SetName(L"Mesh3DIndexBufferResource");
    m_IntermediateIndexBufferResource->SetName(L"Mesh3DIntermediateIndexBufferResource");

    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(WORD));

    if (urgently)
        TransitionResource(commandList, m_IndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);

    m_Initialized = true;
}
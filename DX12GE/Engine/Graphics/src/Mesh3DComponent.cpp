#include "../Mesh3DComponent.h"
#include "../../Base/Application.h"

void Mesh3DComponent::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource>& pDestinationResource, ComPtr<ID3D12Resource>& pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetPrimaryDevice();

    size_t bufferSize = numElements * elementSize;

    bool recreate = false;
    if (pDestinationResource)
    {
        auto desc = pDestinationResource->GetDesc();
        if (desc.Width != bufferSize)
            recreate = true;
    }

    if (!pDestinationResource || recreate)
    {
        auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
        pDestinationResource.Reset();
        ThrowIfFailed(
            device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pDestinationResource)));
    }   

    if (bufferData)
    {
        if (!pIntermediateResource || recreate)
        {
            auto heapPropUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto resourceDescUpload = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
            pIntermediateResource.Reset();
            ThrowIfFailed(
                device->CreateCommittedResource(&heapPropUpload, D3D12_HEAP_FLAG_NONE, &resourceDescUpload, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pIntermediateResource)));
        }        

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(), pDestinationResource.Get(), pIntermediateResource.Get(), 0, 0, 1, &subresourceData);
    }

    device.Reset();
    device = nullptr;
}

void Mesh3DComponent::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D_PRIMITIVE_TOPOLOGY primitiveType)
{
    if (!m_Initialized) return;

    commandList->IASetPrimitiveTopology(primitiveType);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);
    commandList->DrawIndexedInstanced(m_IndiciesCount, 1, 0, 0, 0);
}

void Mesh3DComponent::Destroy()
{
    if (!m_Initialized) return;
    m_Initialized = false;

    m_VertexBuffer.Reset();
    m_VertexBuffer = nullptr;
    m_VertexBufferView = {};
    m_IntermediateVertexBufferResource.Reset();
    m_IntermediateVertexBufferResource = nullptr;

    m_IndexBuffer.Reset();
    m_IndexBuffer = nullptr;
    m_IndexBufferView = {};
    m_IntermediateIndexBufferResource.Reset();
    m_IntermediateIndexBufferResource = nullptr;

    m_IndiciesCount = 0;
    m_VerticesCount = 0;

    if (m_Material && m_Material->CanDrawIt())
    {
        m_Material->Destroy();
    }    
    m_Material = nullptr;
}
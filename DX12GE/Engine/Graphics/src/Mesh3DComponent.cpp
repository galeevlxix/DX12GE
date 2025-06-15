#include "../Mesh3DComponent.h"
#include "../../Base/Application.h"

void Mesh3DComponent::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetPrimaryDevice();

    size_t bufferSize = numElements * elementSize;

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    // Create a committed resource for the GPU resource in a default heap
    ThrowIfFailed(
        device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(pDestinationResource)));

    // Create an committed resource for the upload
    if (bufferData)
    {
        auto r3 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto r4 = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        ThrowIfFailed(
            device->CreateCommittedResource(&r3, D3D12_HEAP_FLAG_NONE, &r4, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(), *pDestinationResource, *pIntermediateResource, 0, 0, 1, &subresourceData);
    }
}

void Mesh3DComponent::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D_PRIMITIVE_TOPOLOGY primitiveType)
{
    if (!Initialized) return;
    commandList->IASetPrimitiveTopology(primitiveType);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);
    commandList->DrawIndexedInstanced(indiciesCount, 1, 0, 0, 0);
}

void Mesh3DComponent::Release()
{
    if (!Initialized) return;
    Initialized = false;
    m_VertexBuffer->Release();
    m_VertexBuffer = nullptr;
    IntermediateVertexBufferResource->Release();
    IntermediateVertexBufferResource = nullptr;
    m_IndexBuffer->Release();
    m_IndexBuffer = nullptr;
    IntermediateIndexBufferResource->Release();
    IntermediateIndexBufferResource = nullptr;
}
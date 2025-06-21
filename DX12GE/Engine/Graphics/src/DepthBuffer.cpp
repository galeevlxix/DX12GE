#include "../DepthBuffer.h"
#include "../../Base/Application.h"
#include "../../Base/DescriptorHeaps.h"
#include "../CascadedShadowMap.h"

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

void DepthBuffer::Init(GraphicsAdapter graphicsAdapter)
{
    Adapter = graphicsAdapter;
    dsvCpuHandleIndex = DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, graphicsAdapter);
}

// Resize the depth buffer to match the size of the client area.
void DepthBuffer::ResizeDepthBuffer(int width, int height)
{
    if (dsvCpuHandleIndex == -1) return;

    // Flush any GPU commands that might be referencing the depth buffer
    Application::Get().Flush();

    width = std::max(1, width);
    height = std::max(1, height);


    ComPtr<ID3D12Device2> device;

    if (Adapter == GraphicAdapterPrimary)
    {
        device = Application::Get().GetPrimaryDevice();
    }
    else
    {
        device = Application::Get().GetSecondDevice();
    }

    // Resize screen dependent resources
    // Create a depth buffer
    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil = { 1.0f, 0 };

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto pDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    ThrowIfFailed(
        device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &pDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &optimizedClearValue, IID_PPV_ARGS(&DepthBufferTexture.m_Resource)));

    // Update the depth-stencil view
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, dsvCpuHandleIndex, Adapter));
    device->CreateDepthStencilView(DepthBufferTexture.m_Resource.Get(), &dsvDesc, dsvHandle);
}

void DepthBuffer::ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, FLOAT depth)
{
    commandList->ClearDepthStencilView(DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, dsvCpuHandleIndex, Adapter), D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

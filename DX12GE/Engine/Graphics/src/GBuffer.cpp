#include "../GBuffer.h"
#include "../../Base/DescriptorHeaps.h"
#include "../ShaderResources.h"

void GBuffer::Init(ComPtr<ID3D12Device2> device, UINT width, UINT height)
{
	m_Device = device;
	m_Width = width;
	m_Height = height;

    BuildResources();
    BuildDescriptors();
}

void GBuffer::Release()
{
    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        m_Targets[i].Texture.Reset();
    }
}

void GBuffer::Resize(UINT width, UINT height)
{
    if (width == m_Width && height == m_Height || m_Device == nullptr)
        return;

    Release();
    Init(m_Device, width, height);
}

void GBuffer::BindRenderTargets(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
{
    const D3D12_CPU_DESCRIPTOR_HANDLE rtvs[GBUFFER_COUNT] =
    {
        m_Targets[0].CpuRtvHandle,
        m_Targets[1].CpuRtvHandle,
        m_Targets[2].CpuRtvHandle,
        m_Targets[3].CpuRtvHandle,
        m_Targets[4].CpuRtvHandle,
    };

    commandList->OMSetRenderTargets(GBUFFER_COUNT, rtvs, FALSE, &dsvHandle);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE GBuffer::SrvGPU(TargetType type) const
{
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Targets[type].GpuSrvHandle);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE GBuffer::RtvCPU(TargetType type) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Targets[type].CpuRtvHandle);
}

void GBuffer::SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    const static FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        TransitionResource(commandList, m_Targets[i].Texture, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandList->ClearRenderTargetView(m_Targets[i].CpuRtvHandle, clearColor, 0, nullptr);
    }
}

void GBuffer::SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        TransitionResource(commandList, m_Targets[i].Texture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
}

void GBuffer::SetGraphicsRootDescriptorTables(int fromSlot, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    for (int i = 0; i < GBUFFER_COUNT; i++)
    {
        commandList->SetGraphicsRootDescriptorTable(
            fromSlot + i,
            DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_Targets[i].SrvHeapIndex));
    }
}

void GBuffer::BuildResources()
{
    static const DXGI_FORMAT formats[GBUFFER_COUNT] = 
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT, // Position
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
        DXGI_FORMAT_R8G8B8A8_UNORM,     // Diffuse
        DXGI_FORMAT_R8G8B8A8_UNORM,     // Emissive
        DXGI_FORMAT_R8G8B8A8_UNORM      // ORM
    };

    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Width = m_Width;
        texDesc.Height = m_Height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = formats[i];
        texDesc.SampleDesc.Count = 1;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clearValue = {};
        clearValue.Format = formats[i];

        CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

        m_Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            &clearValue,
            IID_PPV_ARGS(&m_Targets[i].Texture)
        );
    }
}

void GBuffer::BuildDescriptors()
{
    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        m_Targets[i].SrvHeapIndex = m_Targets[i].SrvHeapIndex == -1 ? DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) : m_Targets[i].SrvHeapIndex;
        m_Targets[i].RtvHeapIndex = m_Targets[i].RtvHeapIndex == -1 ? DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) : m_Targets[i].RtvHeapIndex;

        m_Targets[i].CpuRtvHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_Targets[i].RtvHeapIndex);
        m_Targets[i].CpuSrvHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_Targets[i].SrvHeapIndex);
        m_Targets[i].GpuSrvHandle = DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_Targets[i].SrvHeapIndex);

        m_Device->CreateRenderTargetView(m_Targets[i].Texture.Get(), nullptr, m_Targets[i].CpuRtvHandle);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = m_Targets[i].Texture->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;

        m_Device->CreateShaderResourceView(m_Targets[i].Texture.Get(), &srvDesc, m_Targets[i].CpuSrvHandle);
    }
}

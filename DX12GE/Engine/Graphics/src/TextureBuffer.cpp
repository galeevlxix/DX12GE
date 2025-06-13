#include "../TextureBuffer.h"
#include "../../Base/DescriptorHeaps.h"
#include "../ShaderResources.h"

void TextureBuffer::Init(ComPtr<ID3D12Device2> device, UINT width, UINT height, DXGI_FORMAT format)
{
	m_Device = device;
	m_Width = width;
	m_Height = height;
    m_Format = format;

	BuildResource();
	BuildDescriptors();
}

void TextureBuffer::Release()
{
	Buffer.Reset();
}

void TextureBuffer::Resize(UINT width, UINT height)
{
	if (width == m_Width && height == m_Height || m_Device == nullptr)
		return;

	Release();
	Init(m_Device, width, height, m_Format);
}

void TextureBuffer::BindRenderTarget(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
{
	commandList->OMSetRenderTargets(1, &CpuRtvHandle, FALSE, &dsvHandle);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE TextureBuffer::SrvGPU() const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(GpuSrvHandle);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE TextureBuffer::RtvCPU() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(CpuRtvHandle);
}

void TextureBuffer::SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	TransitionResource(commandList, Buffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ClearRenderTargetView(CpuRtvHandle, clearColor, 0, nullptr);
}

void TextureBuffer::SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	TransitionResource(commandList, Buffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void TextureBuffer::SetGraphicsRootDescriptorTable(int slot, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	commandList->SetGraphicsRootDescriptorTable(slot, DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SrvHeapIndex));
}

void TextureBuffer::BuildResource()
{
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = m_Width;
    texDesc.Height = m_Height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = m_Format;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = m_Format;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        &clearValue,
        IID_PPV_ARGS(&Buffer)
    );
}

void TextureBuffer::BuildDescriptors()
{
    SrvHeapIndex = SrvHeapIndex == -1 ? DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) : SrvHeapIndex;
    RtvHeapIndex = RtvHeapIndex == -1 ? DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) : RtvHeapIndex;

    CpuRtvHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RtvHeapIndex);
    CpuSrvHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SrvHeapIndex);
    GpuSrvHandle = DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SrvHeapIndex);

    m_Device->CreateRenderTargetView(Buffer.Get(), nullptr, CpuRtvHandle);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = Buffer->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    m_Device->CreateShaderResourceView(Buffer.Get(), &srvDesc, CpuSrvHandle);
}

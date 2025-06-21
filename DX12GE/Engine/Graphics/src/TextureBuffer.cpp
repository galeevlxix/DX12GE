#include "../TextureBuffer.h"
#include "../../Base/DescriptorHeaps.h"
#include "../ShaderResources.h"

TextureBuffer::TextureBuffer() { }

void TextureBuffer::Init(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter, UINT width, UINT height, DXGI_FORMAT format)
{
	m_Device = device;
	m_Width = width;
	m_Height = height;
    m_Format = format;
    m_Adapter = graphicsAdapter;

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
    Init(m_Device, m_Adapter, width, height, m_Format);
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
    TransitionResource(commandList, Buffer, currentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
    currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ClearRenderTargetView(CpuRtvHandle, clearColor, 0, nullptr);
}

void TextureBuffer::SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	TransitionResource(commandList, Buffer, currentState, D3D12_RESOURCE_STATE_GENERIC_READ);
    currentState = D3D12_RESOURCE_STATE_GENERIC_READ;
}

void TextureBuffer::SetToCopyDest(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (currentState == D3D12_RESOURCE_STATE_COPY_DEST) return;

    TransitionResource(commandList, Buffer, currentState, D3D12_RESOURCE_STATE_COPY_DEST);
    currentState = D3D12_RESOURCE_STATE_COPY_DEST;
}

void TextureBuffer::SetToCopySource(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (currentState == D3D12_RESOURCE_STATE_COPY_SOURCE) return;

    TransitionResource(commandList, Buffer, currentState, D3D12_RESOURCE_STATE_COPY_SOURCE);
    currentState = D3D12_RESOURCE_STATE_COPY_SOURCE;
}

void TextureBuffer::SetToState(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_RESOURCE_STATES newState)
{
    if (currentState == newState) return;
    TransitionResource(commandList, Buffer, currentState, newState);
    currentState = newState;
}

void TextureBuffer::SetGraphicsRootDescriptorTable(int slot, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	commandList->SetGraphicsRootDescriptorTable(slot, DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SrvHeapIndex, m_Adapter));
}

ComPtr<ID3D12Resource> TextureBuffer::GetResource()
{
    return Buffer;
}

ComPtr<ID3D12Device2> TextureBuffer::GetDevice()
{
    return m_Device;
}

void TextureBuffer::SetResourceName(LPCWSTR name)
{
    m_Name = name;
}

D3D12_RESOURCE_STATES TextureBuffer::GetResourceState()
{
    return currentState;
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

    currentState = D3D12_RESOURCE_STATE_GENERIC_READ;

    m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        currentState,
        &clearValue,
        IID_PPV_ARGS(&Buffer)
    );

    

    if (m_Name != L"") 
        Buffer->SetName(m_Name);
}

void TextureBuffer::BuildDescriptors()
{
    SrvHeapIndex = SrvHeapIndex == -1 ? DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_Adapter) : SrvHeapIndex;
    RtvHeapIndex = RtvHeapIndex == -1 ? DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_Adapter) : RtvHeapIndex;

    CpuRtvHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RtvHeapIndex, m_Adapter);
    CpuSrvHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SrvHeapIndex, m_Adapter);
    GpuSrvHandle = DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SrvHeapIndex, m_Adapter);

    m_Device->CreateRenderTargetView(Buffer.Get(), nullptr, CpuRtvHandle);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = Buffer->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    m_Device->CreateShaderResourceView(Buffer.Get(), &srvDesc, CpuSrvHandle);
}

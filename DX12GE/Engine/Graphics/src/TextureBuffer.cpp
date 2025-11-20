#include "../TextureBuffer.h"
#include "../../Graphics/DescriptorHeaps.h"
#include "../ShaderResources.h"

TextureBuffer::TextureBuffer() { }

void TextureBuffer::Init(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter, UINT width, UINT height, DXGI_FORMAT format, bool createReadback)
{
	m_Device = device;
	m_Width = width;
	m_Height = height;
    m_Format = format;
    m_Adapter = graphicsAdapter;
    m_CreateReadback = createReadback;

	BuildResource();
	BuildDescriptors();
}

void TextureBuffer::Init(ComPtr<ID3D12Device2> device, D3D12_RESOURCE_DESC& resourceDesc, ComPtr<ID3D12Heap> heap, const std::wstring& name, bool createReadback)
{
    m_Device = device;
    m_Width = static_cast<UINT>(resourceDesc.Width);
    m_Height = resourceDesc.Height;
    m_Format = resourceDesc.Format;
    m_CurrentState = D3D12_RESOURCE_STATE_COMMON;
    m_CreateReadback = createReadback;

    ThrowIfFailed(
        m_Device->CreatePlacedResource(
            heap.Get(),
            0,
            &resourceDesc,
            m_CurrentState,
            nullptr,
            IID_PPV_ARGS(&m_Resource)
        )
    );

    m_Name = name.c_str();
    m_Resource->SetName(m_Name);
}

void TextureBuffer::Destroy()
{
	m_Resource.Reset();
    m_Resource = nullptr;

    m_Device.Reset();
    m_Device = nullptr;
}

void TextureBuffer::Resize(UINT width, UINT height)
{
    if ((width == m_Width && height == m_Height) || width == 0 || height == 0 || m_Device == nullptr )
		return;

    m_Resource.Reset();
    m_Resource = nullptr;

    if (m_CreateReadback)
    {
        m_ReadbackBuffer.Reset();
        m_ReadbackBuffer = nullptr;
    }    

    Init(m_Device, m_Adapter, width, height, m_Format, m_CreateReadback);
}

void TextureBuffer::BindRenderTarget(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
{
	commandList->OMSetRenderTargets(1, &m_CpuRtvHandle, FALSE, &dsvHandle);
}

void TextureBuffer::SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (m_CurrentState == D3D12_RESOURCE_STATE_RENDER_TARGET) return;

    TransitionResource(commandList, m_Resource, m_CurrentState, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    commandList->ClearRenderTargetView(m_CpuRtvHandle, clearColor, 0, nullptr);
}

void TextureBuffer::SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (m_CurrentState == D3D12_RESOURCE_STATE_GENERIC_READ) return;

	TransitionResource(commandList, m_Resource, m_CurrentState, D3D12_RESOURCE_STATE_GENERIC_READ);
    m_CurrentState = D3D12_RESOURCE_STATE_GENERIC_READ;
}

void TextureBuffer::SetToCopyDest(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (m_CurrentState == D3D12_RESOURCE_STATE_COPY_DEST) return;

    TransitionResource(commandList, m_Resource, m_CurrentState, D3D12_RESOURCE_STATE_COPY_DEST);
    m_CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
}

void TextureBuffer::SetToCopySource(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (m_CurrentState == D3D12_RESOURCE_STATE_COPY_SOURCE) return;

    TransitionResource(commandList, m_Resource, m_CurrentState, D3D12_RESOURCE_STATE_COPY_SOURCE);
    m_CurrentState = D3D12_RESOURCE_STATE_COPY_SOURCE;
}

void TextureBuffer::SetToState(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_RESOURCE_STATES newState)
{
    if (m_CurrentState == newState) return;

    TransitionResource(commandList, m_Resource, m_CurrentState, newState);
    m_CurrentState = newState;
}

void TextureBuffer::SetGraphicsRootDescriptorTable(int slot, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	commandList->SetGraphicsRootDescriptorTable(
        slot, 
        DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 
            m_SrvHeapIndex, 
            m_Adapter));
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

    m_CurrentState = D3D12_RESOURCE_STATE_GENERIC_READ;

    ThrowIfFailed(
        m_Device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            m_CurrentState,
            &clearValue,
            IID_PPV_ARGS(&m_Resource)
        ));

    if (m_Name != L"") 
        ThrowIfFailed(m_Resource->SetName(m_Name));

    if (!m_CreateReadback) return;

    D3D12_RESOURCE_DESC readbackDesc = {};
    readbackDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    readbackDesc.Alignment = 0;

    UINT64 totalBytes = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    UINT numRows;
    UINT64 rowSize;
    m_Device->GetCopyableFootprints(
        &texDesc,
        0, 1, 0,
        &footprint,
        &numRows,
        &rowSize,
        &totalBytes
    );
    readbackDesc.Width = totalBytes;

    readbackDesc.Height = 1;
    readbackDesc.DepthOrArraySize = 1;
    readbackDesc.MipLevels = 1;
    readbackDesc.Format = DXGI_FORMAT_UNKNOWN;
    readbackDesc.SampleDesc.Count = 1;
    readbackDesc.SampleDesc.Quality = 0;
    readbackDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    readbackDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES readbackHeapProps(D3D12_HEAP_TYPE_READBACK);

    ThrowIfFailed(
        m_Device->CreateCommittedResource(
            &readbackHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &readbackDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_ReadbackBuffer)
        ));
}

void TextureBuffer::BuildDescriptors()
{
    m_SrvHeapIndex = m_SrvHeapIndex == -1 ? DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_Adapter) : m_SrvHeapIndex;
    m_RtvHeapIndex = m_RtvHeapIndex == -1 ? DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_Adapter) : m_RtvHeapIndex;

    m_CpuRtvHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_RtvHeapIndex, m_Adapter);
    m_CpuSrvHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_SrvHeapIndex, m_Adapter);
    m_GpuSrvHandle = DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_SrvHeapIndex, m_Adapter);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = m_Format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    m_Device->CreateRenderTargetView(m_Resource.Get(), &rtvDesc, m_CpuRtvHandle);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;

    m_Device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, m_CpuSrvHandle);
}

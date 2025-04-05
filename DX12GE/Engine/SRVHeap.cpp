#include "SRVHeap.h"
#include "Application.h"

static SRVHeap* gs_pSingelton = nullptr;

SRVHeap::SRVHeap()
{
    m_Device = Application::Get().GetDevice();

    m_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = 77;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    ThrowIfFailed(
        m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SRVHeap)));
}

void SRVHeap::Create()
{
    if (!gs_pSingelton)
    {
        gs_pSingelton = new SRVHeap();
    }
}

SRVHeap* SRVHeap::GetHeap()
{
    assert(gs_pSingelton);
    return gs_pSingelton;
}

void SRVHeap::AddResource(ComPtr<ID3D12Resource> resource, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc)
{
    static CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
    m_Device->CreateShaderResourceView(resource.Get(), &srvDesc, handle);
    handle.Offset(1, m_DescriptorSize);
}

void SRVHeap::Set(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetDescriptorHeaps(1, m_SRVHeap.GetAddressOf());
}



#include "../CrossAdapterResource.h"

UINT Align(const UINT size, const UINT alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

CrossAdapterResource::CrossAdapterResource(
	ComPtr<ID3D12Device2> primaryDevice, 
	ComPtr<ID3D12Device2> secondDevice, 
	D3D12_RESOURCE_DESC& resourceDesc,
	const std::wstring& name)
{
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
    UINT64 sizeInBytes;
    UINT64 totalBytes;
    primaryDevice->GetCopyableFootprints(&resourceDesc, 0, 1, 0, &layout, nullptr, &sizeInBytes, &totalBytes);
    UINT64 heapSize = Align(layout.Footprint.RowPitch * layout.Footprint.Height);
    heapSize = std::max(heapSize, totalBytes);

    // общая куча для двух адаптеров
    CD3DX12_HEAP_DESC heapDesc(
        heapSize,
        D3D12_HEAP_TYPE_DEFAULT,
        0,
        D3D12_HEAP_FLAG_SHARED | D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER);

    ThrowIfFailed(primaryDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&crossAdapterResourceHeap[0])));

    HANDLE heapHandle = nullptr;
    ThrowIfFailed(primaryDevice->CreateSharedHandle(crossAdapterResourceHeap[0].Get(), nullptr, GENERIC_ALL, nullptr, &heapHandle));

    HRESULT openSharedHandleResult = secondDevice->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&crossAdapterResourceHeap[1]));

    CloseHandle(heapHandle);

    ThrowIfFailed(openSharedHandleResult);

    m_PrimeResource = std::make_shared<TextureBuffer>();
    m_PrimeResource->Init(primaryDevice, resourceDesc, crossAdapterResourceHeap[0], name);
    m_PrimeResource->SetAdapter(GraphicAdapterPrimary);

    m_SharedResource = std::make_shared<TextureBuffer>();
    m_SharedResource->Init(secondDevice, resourceDesc, crossAdapterResourceHeap[1], name + L" Shared");
    m_SharedResource->SetAdapter(GraphicAdapterSecond);

    isInit = true;
}

std::shared_ptr<TextureBuffer> CrossAdapterResource::GetPrimeResource()
{
    return m_PrimeResource;
}

std::shared_ptr<TextureBuffer> CrossAdapterResource::GetSharedResource()
{
    return m_SharedResource;
}

void CrossAdapterResource::Resize(const UINT newWidth, const UINT newHeight)
{
    auto desc = m_PrimeResource->GetResource()->GetDesc();

    if ((desc.Width == newWidth && desc.Height == newHeight) ||
        newWidth == 0 ||
        newHeight == 0 ||
        m_PrimeResource->GetDevice() == nullptr)
        return;

    desc.Width = newWidth;
    desc.Height = newHeight;

    m_PrimeResource->Destroy();
    m_PrimeResource->Init(
        m_PrimeResource->GetDevice(), 
        desc, 
        crossAdapterResourceHeap[0], 
        m_PrimeResource->GetName());

    m_SharedResource->Destroy();
    m_SharedResource->Init(
        m_SharedResource->GetDevice(), 
        desc, 
        crossAdapterResourceHeap[1], 
        m_SharedResource->GetName());
}

void CrossAdapterResource::Destroy()
{
    m_PrimeResource->Destroy();
    m_PrimeResource = nullptr;
    
    m_SharedResource->Destroy();
    m_SharedResource = nullptr;

    crossAdapterResourceHeap[0].Reset();
    crossAdapterResourceHeap[0] = nullptr;

    crossAdapterResourceHeap[1].Reset();
    crossAdapterResourceHeap[1] = nullptr;
}

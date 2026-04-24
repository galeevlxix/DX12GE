#include "../CrossAdapterResource.h"

UINT Align(const UINT size, const UINT alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

CrossAdapterResource::CrossAdapterResource(
	ComPtr<ID3D12Device2> primaryDevice, 
	ComPtr<ID3D12Device2> secondDevice, 
	D3D12_RESOURCE_DESC resourceDesc,
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

    ThrowIfFailed(primaryDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_CrossAdapterResourceHeap[0])));

    HANDLE heapHandle = nullptr;
    ThrowIfFailed(
        primaryDevice->CreateSharedHandle(m_CrossAdapterResourceHeap[0].Get(), nullptr, GENERIC_ALL, nullptr, &heapHandle));
    ThrowIfFailed(
        secondDevice->OpenSharedHandle(heapHandle, IID_PPV_ARGS(&m_CrossAdapterResourceHeap[1])));
    CloseHandle(heapHandle);

    m_PrimarySharedResource = std::make_shared<TextureBuffer>();
    m_PrimarySharedResource->Init(primaryDevice, resourceDesc, m_CrossAdapterResourceHeap[0], name + L"Cross Adapter Primary", false);
    m_PrimarySharedResource->SetAdapter(GraphicAdapterPrimary);

    m_SecondSharedResource = std::make_shared<TextureBuffer>();
    m_SecondSharedResource->Init(secondDevice, resourceDesc, m_CrossAdapterResourceHeap[1], name + L" Cross Adapter Second", false);
    m_SecondSharedResource->SetAdapter(GraphicAdapterSecond);
}

std::shared_ptr<TextureBuffer> CrossAdapterResource::GetPrimarySharedResource()
{
    return m_PrimarySharedResource;
}

std::shared_ptr<TextureBuffer> CrossAdapterResource::GetSecondSharedResource()
{
    return m_SecondSharedResource;
}

void CrossAdapterResource::Resize(const UINT newWidth, const UINT newHeight)
{
    auto desc = m_PrimarySharedResource->GetResource()->GetDesc();

    if ((desc.Width == newWidth && desc.Height == newHeight) ||
        newWidth == 0 ||
        newHeight == 0 ||
        m_PrimarySharedResource->GetDevice() == nullptr)
        return;

    desc.Width = newWidth;
    desc.Height = newHeight;

    m_PrimarySharedResource->Destroy();
    m_PrimarySharedResource->Init(
        m_PrimarySharedResource->GetDevice(), 
        desc, 
        m_CrossAdapterResourceHeap[0], 
        m_PrimarySharedResource->GetName(), 
        false);

    m_SecondSharedResource->Destroy();
    m_SecondSharedResource->Init(
        m_SecondSharedResource->GetDevice(), 
        desc, 
        m_CrossAdapterResourceHeap[1], 
        m_SecondSharedResource->GetName(), 
        false);
}

void CrossAdapterResource::Destroy()
{
    m_PrimarySharedResource->Destroy();
    m_PrimarySharedResource = nullptr;
    
    m_SecondSharedResource->Destroy();
    m_SecondSharedResource = nullptr;

    m_CrossAdapterResourceHeap[0].Reset();
    m_CrossAdapterResourceHeap[0] = nullptr;

    m_CrossAdapterResourceHeap[1].Reset();
    m_CrossAdapterResourceHeap[1] = nullptr;
}

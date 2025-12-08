#include "../TextureComponent.h"

#include "../../Base/Application.h"
#include "../../Graphics/DescriptorHeaps.h"
#include "../ShaderResources.h"
#include "../GBuffer.h"
#include <DirectXTex.h>

using namespace DirectX;

void TextureComponent::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, string path)
{
    auto device = Application::Get().GetPrimaryDevice();

    ScratchImage image;
    ScratchImage mipChain;

    ThrowIfFailed(
        LoadFromWICFile(
            (wstring(path.begin(), path.end())).c_str(),
            WIC_FLAGS_FORCE_RGB,
            nullptr,
            image));

    ThrowIfFailed(
        GenerateMipMaps(
            *image.GetImages(),
            TEX_FILTER_BOX,
            0,
            mipChain));

    // create resource 
    const auto& chainBase = *mipChain.GetImages();

    D3D12_RESOURCE_DESC texDesc;
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = (UINT)chainBase.width;
    texDesc.Height = (UINT)chainBase.height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = (UINT16)mipChain.GetImageCount();
    texDesc.Format = chainBase.format;
    texDesc.SampleDesc = DXGI_SAMPLE_DESC();
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    texDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

    CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };

    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_Resource)));

    // collect subresource data
    vector<D3D12_SUBRESOURCE_DATA> subresources((int)mipChain.GetImageCount());
    for (int i = 0; i < (int)mipChain.GetImageCount(); i++)
    {
        const auto img = mipChain.GetImage(i, 0, 0);
        auto& subresource = subresources[i];
        subresource.pData = img->pixels;
        subresource.RowPitch = (LONG_PTR)img->rowPitch;
        subresource.SlicePitch = (LONG_PTR)img->slicePitch;
    }

    // create the intermediate upload buffer
    CD3DX12_HEAP_PROPERTIES heapProps2{ D3D12_HEAP_TYPE_UPLOAD };
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Resource.Get(), 0, static_cast<uint32_t>(subresources.size()));
    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProps2,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_UploadBuffer)));

    // write commands to copy data to upload texture (copying each subresource)
    UpdateSubresources(
        commandList.Get(),
        m_Resource.Get(),
        m_UploadBuffer.Get(),
        0,
        0,
        static_cast<uint32_t>(subresources.size()),
        subresources.data());

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_Resource->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = m_Resource->GetDesc().MipLevels;

    m_SRVHeapIndex = DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GraphicAdapterPrimary);

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
        DescriptorHeaps::GetCPUHandle(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            m_SRVHeapIndex, GraphicAdapterPrimary)
    );

    device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, handle);

    device.Reset();
    device = nullptr;

    m_Initialized = true;
}

void TextureComponent::OnLoadCubemap(ComPtr<ID3D12GraphicsCommandList2> commandList, string path)
{
    auto device = Application::Get().GetPrimaryDevice();

    // Загружаем DDS кубическую текстуру
    ScratchImage image;
    ThrowIfFailed(
        LoadFromDDSFile(
            std::wstring(path.begin(), path.end()).c_str(),
            DDS_FLAGS_FORCE_RGB,
            nullptr,
            image));

    const TexMetadata& metadata = image.GetMetadata();

    if (!(metadata.miscFlags & TEX_MISC_TEXTURECUBE))
        throw std::runtime_error("Файл не является кубической текстурой (cubemap DDS)");

    // Создаем mip chain, если нет
    ScratchImage mipChain;
    if (metadata.mipLevels == 1)
    {
        ThrowIfFailed(
            GenerateMipMaps(
                *image.GetImages(),
                TEX_FILTER_DEFAULT,
                0,
                mipChain));
    }
    else
    {
        mipChain = std::move(image);
    }

    const TexMetadata& mipMetadata = mipChain.GetMetadata();

    // Создаём ресурс
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = static_cast<UINT>(mipMetadata.width);
    texDesc.Height = static_cast<UINT>(mipMetadata.height);
    texDesc.DepthOrArraySize = static_cast<UINT16>(mipMetadata.arraySize); // должно быть 6
    texDesc.MipLevels = static_cast<UINT16>(mipMetadata.mipLevels);
    texDesc.Format = mipMetadata.format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_Resource)));

    // Готовим подресурсы (все 6 граней * мипы)
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    for (int face = 0; face < 6; ++face)
    {
        for (int mip = 0; mip < (int)metadata.mipLevels; ++mip)
        {
            const Image* img = mipChain.GetImage(mip, face, 0);
            D3D12_SUBRESOURCE_DATA s = {};
            s.pData = img->pixels;
            s.RowPitch = img->rowPitch;
            s.SlicePitch = img->slicePitch;
            subresources.push_back(s);
        }
    }

    //PrepareUpload(device.Get(), mipChain.GetImages(), mipChain.GetImageCount(), mipMetadata, subresources);

    // Создаём промежуточный upload buffer
    UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Resource.Get(), 0, static_cast<UINT>(subresources.size()));

    CD3DX12_HEAP_PROPERTIES heapPropsUpload(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapPropsUpload,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_UploadBuffer)));

    // Копируем данные в GPU ресурс
    UpdateSubresources(
        commandList.Get(),
        m_Resource.Get(),
        m_UploadBuffer.Get(),
        0,
        0,
        static_cast<UINT>(subresources.size()),
        subresources.data());

    // Синхронизация состояния
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_Resource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    commandList->ResourceBarrier(1, &barrier);

    // Создаём SRV для cubemap
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = mipMetadata.format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.TextureCube.MipLevels = static_cast<UINT>(mipMetadata.mipLevels);
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

    m_SRVHeapIndex = DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GraphicAdapterPrimary);

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
        DescriptorHeaps::GetCPUHandle(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            m_SRVHeapIndex,
            GraphicAdapterPrimary)
    );

    device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, handle);

    device.Reset();
    device = nullptr;

    m_Initialized = true;
}

void TextureComponent::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot)
{
    commandList->SetGraphicsRootDescriptorTable(
        slot,
        DescriptorHeaps::GetGPUHandle(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 
            m_SRVHeapIndex, GraphicAdapterPrimary)
    );
}

void TextureComponent::Destroy()
{
    if (!m_Initialized) return;
    m_Initialized = false;

    m_Resource.Reset();
    m_Resource = nullptr;

    m_UploadBuffer.Reset();
    m_UploadBuffer = nullptr;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureComponent::GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    return DescriptorHeaps::GetCPUHandle(heapType, m_SRVHeapIndex, GraphicAdapterPrimary);
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureComponent::GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    return DescriptorHeaps::GetGPUHandle(heapType, m_SRVHeapIndex, GraphicAdapterPrimary);
}

bool TextureComponent::IsInitialized()
{
    return m_Initialized;
}

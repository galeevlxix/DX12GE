#include "Application.h"
#include "Texture.h"
#include <DirectXTex.h>
#include "DescriptorHeaps.h"
#include "ShaderResources.h"
#include "GBuffer.h"
#include "../Game/BaseObject.h"

using namespace DirectX;

void Texture::Load(ComPtr<ID3D12GraphicsCommandList2> commandList, string path)
{
    auto device = Application::Get().GetDevice();

    ScratchImage image;
    TexMetadata metadata;
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

    CreateShaderResourceView(srvDesc);
}

void Texture::CreateShaderResourceView(D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc)
{
    auto device = Application::Get().GetDevice();

    m_SRVHeapIndex = DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
        DescriptorHeaps::GetCPUHandle(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            m_SRVHeapIndex)
    );

    device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, handle);
}

void Texture::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot)
{
    commandList->SetGraphicsRootDescriptorTable(
        slot,
        DescriptorHeaps::GetGPUHandle(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 
            m_SRVHeapIndex)
    );
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    return DescriptorHeaps::GetCPUHandle(heapType, m_SRVHeapIndex);
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture::GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    return DescriptorHeaps::GetGPUHandle(heapType, m_SRVHeapIndex);
}

//zaprogramirui mne pojaluista sistemu chastiz v korobke 
#include "Material.h"
#include "Application.h"
#include <DirectXTex.h>

using namespace DirectX;

void Material::Load(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    auto device = Application::Get().GetDevice();

    // load image data from disk
    string path = " ";
    if (m_ImagePaths.size() != 0)
    {
        path = m_ImagePaths[TextureType::DIFFUSE];
    }
    else
    {
        DrawIt = false;
        return;
    }

    ScratchImage image;
    TexMetadata metadata;
    ThrowIfFailed(
        LoadFromWICFile(
            (wstring(path.begin(), path.end())).c_str(),
            WIC_FLAGS_FORCE_RGB,
            nullptr,
            image));

    // generate mip chain
    ScratchImage mipChain;
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
    texDesc.MipLevels = 0; // (UINT16)mipChain.GetImageCount();
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
            IID_PPV_ARGS(&m_Texture)));

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
    //CD3DX12_HEAP_PROPERTIES heapProps2{ D3D12_HEAP_TYPE_UPLOAD };
    CD3DX12_HEAP_PROPERTIES heapProps2{ D3D12_HEAP_TYPE_UPLOAD };
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, static_cast<uint32_t>(subresources.size()));
    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProps2,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer)));

    // write commands to copy data to upload texture (copying each subresource)
    UpdateSubresources(
        commandList.Get(),
        m_Texture.Get(),
        uploadBuffer.Get(),
        0,
        0,
        static_cast<uint32_t>(subresources.size()),
        subresources.data());

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier);

    // DESCRIPTOR HEAP FOR SHADER RESOURCE VIEW
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = { };
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(
        device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SRVHeap)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap.Get()->GetCPUDescriptorHandleForHeapStart());

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
    srvDesc.Format = m_Texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = m_Texture->GetDesc().MipLevels;

    device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, srvHandle);
}

void Material::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetDescriptorHeaps(1, m_SRVHeap.GetAddressOf());
    commandList->SetGraphicsRootDescriptorTable(1, m_SRVHeap.Get()->GetGPUDescriptorHandleForHeapStart());
}
#include "ShadowMap.h"
#include "../DescriptorHeaps.h"

ShadowMap::ShadowMap(ComPtr<ID3D12Device2> device, UINT width, UINT height)
{
    md3dDevice = device;
    mWidth = width;
    mHeight = height;

    mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    mScissorRect = { 0, 0, (int)width, (int)height };

    BuildResource();
    BuildDescriptors();
}

UINT ShadowMap::Width() const
{
    return mWidth;
}

UINT ShadowMap::Height() const
{
    return mHeight;
}

ID3D12Resource* ShadowMap::Resource()
{
    return mTexture.m_Resource.Get();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ShadowMap::Srv() const
{
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(mhGpuSrv);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ShadowMap::Dsv() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(mhCpuDsv);
}

D3D12_VIEWPORT ShadowMap::Viewport() const
{
    return mViewport;
}

D3D12_RECT ShadowMap::ScissorRect() const
{
    return mScissorRect;
}

void ShadowMap::OnResize(UINT newWidth, UINT newHeight)
{
    if ((mWidth != newWidth) || (mHeight != newHeight))
    {
        mWidth = newWidth;
        mHeight = newHeight;

        BuildResource();
        BuildDescriptors();
    }    
}

void ShadowMap::SetToWrite(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    //commandList->SetDescriptorHeaps(1, m_SRVHeap.GetAddressOf());
    commandList->SetGraphicsRootDescriptorTable(1, mhGpuSrv);
}

void ShadowMap::SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetGraphicsRootDescriptorTable(1, mhGpuSrv);
}

void ShadowMap::BuildDescriptors()
{
    // Create SRV to resource so we can sample the shadow map in a shader program.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    srvDesc.Texture2D.PlaneSlice = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));
    md3dDevice->CreateShaderResourceView(mTexture.m_Resource.Get(), &srvDesc, srvHandle);

    // Create DSV to resource so we can render to the shadow map.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.Texture2D.MipSlice = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 0));
    md3dDevice->CreateDepthStencilView(mTexture.m_Resource.Get(), &dsvDesc, dsvHandle);

    mhCpuSrv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0);
    mhGpuSrv = DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0);
    mhCpuDsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 0);
}

void ShadowMap::BuildResource()
{
    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mWidth;
    texDesc.Height = mHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    ThrowIfFailed(
        md3dDevice->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            &optClear,
            IID_PPV_ARGS(&(mTexture.m_Resource))
        )
    );
}

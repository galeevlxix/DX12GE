#include "../SSRCrossAdapterResources.h"

std::shared_ptr<CrossAdapterResource> CrossAdapterTextureResources::CreateCrossResource(std::shared_ptr<TextureBuffer> buffer, const std::wstring& name)
{
    auto desc = buffer->GetResource()->GetDesc();
    return std::make_shared<CrossAdapterResource>(PrimaryDevice, SecondDevice, desc, name);
}

void CrossAdapterTextureResources::CopyResource(ComPtr<ID3D12GraphicsCommandList2> commandList, std::shared_ptr<TextureBuffer> src, std::shared_ptr<TextureBuffer> dst, bool toShared)
{
    D3D12_RESOURCE_STATES oldState = toShared ? src->GetResourceState() : dst->GetResourceState();

    dst->SetToCopyDest(commandList); 
    src->SetToCopySource(commandList);
    commandList->CopyResource(dst->GetResource().Get(), src->GetResource().Get());

    if (toShared) src->SetToState(commandList, oldState);
    else dst->SetToState(commandList, oldState);
}

void CrossAdapterTextureResources::Initialize()
{
    PositionBuffer = CreateCrossResource(   PrimaryGBuffer->GetBuffer(GBuffer::POSITION),   L"Shared GBuffer Position");
    NormalBuffer = CreateCrossResource(     PrimaryGBuffer->GetBuffer(GBuffer::NORMAL),     L"Shared GBuffer Normal");
    ORMBuffer = CreateCrossResource(        PrimaryGBuffer->GetBuffer(GBuffer::ORM),        L"Shared GBuffer ORM");
    LightPassBuffer = CreateCrossResource(  PrimaryLightPassResult,                         L"Shared Light Pass Result");
    SSRBuffer = CreateCrossResource(        PrimarySSRResult,                               L"Shared SSR Result");
}

void CrossAdapterTextureResources::CopyPrimaryDeviceDataToSharedMemory(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::POSITION), PositionBuffer->GetPrimeResource(),   true);
    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::NORMAL),   NormalBuffer->GetPrimeResource(),     true);
    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::ORM),      ORMBuffer->GetPrimeResource(),        true);
    CopyResource(commandList, PrimaryLightPassResult,                       LightPassBuffer->GetPrimeResource(),  true);
}

void CrossAdapterTextureResources::CopySharedMemoryDataToPrimaryDevice(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CopyResource(commandList, SSRBuffer->GetPrimeResource(), PrimarySSRResult, false);
}

void CrossAdapterTextureResources::CopySecondDeviceDataToSharedMemory(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CopyResource(commandList, SecondSSRResult, SSRBuffer->GetSharedResource(), true);
}

void CrossAdapterTextureResources::CopySharedMemoryDataToSecondDevice(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CopyResource(commandList, PositionBuffer->GetSharedResource(),  SecondGBuffer->GetBuffer(GBuffer::POSITION),  false);
    CopyResource(commandList, NormalBuffer->GetSharedResource(),    SecondGBuffer->GetBuffer(GBuffer::NORMAL),    false);
    CopyResource(commandList, ORMBuffer->GetSharedResource(),       SecondGBuffer->GetBuffer(GBuffer::ORM),       false);
    CopyResource(commandList, LightPassBuffer->GetSharedResource(), SecondLightPassResult,                        false);
}

void CrossAdapterTextureResources::Resize(UINT width, UINT height)
{
    PrimaryDepthBuffer->ResizeDepthBuffer(width, height);
    SecondDepthBuffer->ResizeDepthBuffer(width, height);

    PrimaryGBuffer->Resize(width, height);
    PrimarySSRResult->Resize(width, height);
    PrimaryLightPassResult->Resize(width, height);

    SecondGBuffer->Resize(width, height);
    SecondSSRResult->Resize(width, height);
    SecondLightPassResult->Resize(width, height);

    PositionBuffer->Resize(width, height);
    NormalBuffer->Resize(width, height);
    ORMBuffer->Resize(width, height);
    LightPassBuffer->Resize(width, height);
    SSRBuffer->Resize(width, height);
}

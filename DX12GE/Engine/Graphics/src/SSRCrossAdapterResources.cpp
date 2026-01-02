#include "../SSRCrossAdapterResources.h"
#include "../../Base/CommandQueue.h"

std::shared_ptr<CrossAdapterResource> CrossAdapterTextureResources::CreateCrossResource(std::shared_ptr<TextureBuffer> buffer, const std::wstring& name)
{
    auto desc = buffer->GetResource()->GetDesc();
    return std::make_shared<CrossAdapterResource>(PrimaryDevice, SecondDevice, desc, name);
}

void CrossAdapterTextureResources::Initialize()
{
    PositionBuffer = CreateCrossResource(PrimaryGBuffer->GetBuffer(GBuffer::POSITION), L"Shared GBuffer Position");
    NormalBuffer = CreateCrossResource(PrimaryGBuffer->GetBuffer(GBuffer::NORMAL), L"Shared GBuffer Normal");
    ORMBuffer = CreateCrossResource(PrimaryGBuffer->GetBuffer(GBuffer::ORM), L"Shared GBuffer ORM");
    LightPassBuffer = CreateCrossResource(PrimaryLightPassResult, L"Shared Light Pass Result");
    SSRBuffer = CreateCrossResource(PrimarySSRResult, L"Shared SSR Result");
}

void CrossAdapterTextureResources::SetStatesToPrimaryResources(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    PositionBuffer->GetPrimeResource()->SetToCopyDest(commandList);
    NormalBuffer->GetPrimeResource()->SetToCopyDest(commandList);
    ORMBuffer->GetPrimeResource()->SetToCopyDest(commandList);
    LightPassBuffer->GetPrimeResource()->SetToCopyDest(commandList);
    SSRBuffer->GetPrimeResource()->SetToCopySource(commandList);

    primaryStates = true;
}

void CrossAdapterTextureResources::SetStatesToSharedResources(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    PositionBuffer->GetSharedResource()->SetToCopySource(commandList);
    NormalBuffer->GetSharedResource()->SetToCopySource(commandList);
    ORMBuffer->GetSharedResource()->SetToCopySource(commandList);
    LightPassBuffer->GetSharedResource()->SetToCopySource(commandList);
    SSRBuffer->GetSharedResource()->SetToCopyDest(commandList);

    secondStates = true;
}

void CrossAdapterTextureResources::CopyResource(ComPtr<ID3D12GraphicsCommandList2> commandList, std::shared_ptr<TextureBuffer> src, std::shared_ptr<TextureBuffer> dst, bool toShared)
{
    if (toShared)
    {
        src->SetToCopySource(commandList);
    }
    else
    {
        dst->SetToCopyDest(commandList);
    }

    commandList->CopyResource(dst->GetResource().Get(), src->GetResource().Get());
}

void CrossAdapterTextureResources::CopyPrimaryDeviceDataToSharedMemory(TestTime* test)
{
    test->BeginPass();

    std::shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList(); 

    if (!primaryStates)
        SetStatesToPrimaryResources(commandList);

    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::POSITION), PositionBuffer->GetPrimeResource(),   true);
    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::NORMAL),   NormalBuffer->GetPrimeResource(),     true);
    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::ORM),      ORMBuffer->GetPrimeResource(),        true);
    CopyResource(commandList, PrimaryLightPassResult,                       LightPassBuffer->GetPrimeResource(),  true);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::CopyPrimaryDeviceDataToSharedMemory);
}

void CrossAdapterTextureResources::CopySharedMemoryDataToPrimaryDevice(TestTime* test)
{
    test->BeginPass();

    std::shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    if (!primaryStates)
        SetStatesToPrimaryResources(commandList);

    CopyResource(commandList, SSRBuffer->GetPrimeResource(), PrimarySSRResult, false);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::CopySharedMemoryDataToPrimaryDevice);
}

void CrossAdapterTextureResources::CopySecondDeviceDataToSharedMemory(TestTime* test)
{
    test->BeginPass();

    std::shared_ptr<CommandQueue> commandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    if (!secondStates)
        SetStatesToSharedResources(commandList);

    CopyResource(commandList, SecondSSRResult, SSRBuffer->GetSharedResource(), true);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::CopySecondDeviceDataToSharedMemory);
}

void CrossAdapterTextureResources::CopySharedMemoryDataToSecondDevice(TestTime* test)
{
    test->BeginPass();

    std::shared_ptr<CommandQueue> commandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    if (!secondStates)
        SetStatesToSharedResources(commandList);

    CopyResource(commandList, PositionBuffer->GetSharedResource(),  SecondGBuffer->GetBuffer(GBuffer::POSITION),  false);
    CopyResource(commandList, NormalBuffer->GetSharedResource(),    SecondGBuffer->GetBuffer(GBuffer::NORMAL),    false);
    CopyResource(commandList, ORMBuffer->GetSharedResource(),       SecondGBuffer->GetBuffer(GBuffer::ORM),       false);
    CopyResource(commandList, LightPassBuffer->GetSharedResource(), SecondLightPassResult,                        false);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::CopySharedMemoryDataToSecondDevice);
}

void CrossAdapterTextureResources::Resize(UINT width, UINT height)
{
    PrimaryDepthBuffer->Resize(width, height);
    SecondDepthBuffer->Resize(width, height);

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

void CrossAdapterTextureResources::Destroy()
{
    PositionBuffer->Destroy();
    PositionBuffer.reset();
    PositionBuffer = nullptr;

    NormalBuffer->Destroy();
    NormalBuffer.reset();
    NormalBuffer = nullptr;

    ORMBuffer->Destroy();
    ORMBuffer.reset();
    ORMBuffer = nullptr;

    LightPassBuffer->Destroy();
    LightPassBuffer.reset();
    LightPassBuffer = nullptr;

    SSRBuffer->Destroy();
    SSRBuffer.reset();
    SSRBuffer = nullptr;


    PrimaryDepthBuffer->Destroy();
    PrimaryDepthBuffer.reset();
    PrimaryDepthBuffer = nullptr;

    SecondDepthBuffer->Destroy();
    SecondDepthBuffer.reset();
    SecondDepthBuffer = nullptr;


    PrimaryGBuffer->Destroy();
    PrimaryGBuffer.reset();
    PrimaryGBuffer = nullptr;

    PrimaryLightPassResult->Destroy();
    PrimaryLightPassResult.reset();
    PrimaryLightPassResult = nullptr;

    PrimarySSRResult->Destroy();
    PrimarySSRResult.reset();
    PrimarySSRResult = nullptr;


    SecondGBuffer->Destroy();
    SecondGBuffer.reset();
    SecondGBuffer = nullptr;

    SecondLightPassResult->Destroy();
    SecondLightPassResult.reset();
    SecondLightPassResult = nullptr;

    SecondSSRResult->Destroy();
    SecondSSRResult.reset();
    SecondSSRResult = nullptr;


    PrimaryDevice.Reset();
    PrimaryDevice = nullptr;

    SecondDevice.Reset();
    SecondDevice = nullptr;
}
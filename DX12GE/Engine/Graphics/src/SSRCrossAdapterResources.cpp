#include "../SSRCrossAdapterResources.h"
#include "../../Base/CommandQueue.h"

std::shared_ptr<CrossAdapterResource> CrossAdapterTextureResources::CreateCrossResource(std::shared_ptr<TextureBuffer> buffer, const std::wstring& name)
{
    auto desc = buffer->GetResource()->GetDesc();
    return std::make_shared<CrossAdapterResource>(m_PrimaryDevice, m_SecondDevice, desc, name);
}

void CrossAdapterTextureResources::Initialize()
{
    m_PrimaryDevice = Application::Get().GetPrimaryDevice();
    m_SecondDevice = Application::Get().GetSecondDevice();

    m_PrimaryCopyCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    m_SecondCopyCommandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

    m_SharedPositionBuffer = CreateCrossResource(PrimaryGBuffer->GetBuffer(GBuffer::POSITION), L"Shared GBuffer Position");
    m_SharedNormalBuffer = CreateCrossResource(PrimaryGBuffer->GetBuffer(GBuffer::NORMAL), L"Shared GBuffer Normal");
    m_SharedORMBuffer = CreateCrossResource(PrimaryGBuffer->GetBuffer(GBuffer::ORM), L"Shared GBuffer ORM");
    m_SharedLightPassBuffer = CreateCrossResource(PrimaryLightPassResult, L"Shared Light Pass Result");
    m_SharedSSRBuffer = CreateCrossResource(PrimarySSRResult, L"Shared SSR Result");
}

void CrossAdapterTextureResources::SetStatesToPrimaryResources(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    m_SharedPositionBuffer->GetPrimarySharedResource()->SetToCopyDest(commandList);
    m_SharedNormalBuffer->GetPrimarySharedResource()->SetToCopyDest(commandList);
    m_SharedORMBuffer->GetPrimarySharedResource()->SetToCopyDest(commandList);
    m_SharedLightPassBuffer->GetPrimarySharedResource()->SetToCopyDest(commandList);
    m_SharedSSRBuffer->GetPrimarySharedResource()->SetToCopySource(commandList);

    m_PrimaryStatesAreReady = true;
}

void CrossAdapterTextureResources::SetStatesToSharedResources(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    m_SharedPositionBuffer->GetSecondSharedResource()->SetToCopySource(commandList);
    m_SharedNormalBuffer->GetSecondSharedResource()->SetToCopySource(commandList);
    m_SharedORMBuffer->GetSecondSharedResource()->SetToCopySource(commandList);
    m_SharedLightPassBuffer->GetSecondSharedResource()->SetToCopySource(commandList);
    m_SharedSSRBuffer->GetSecondSharedResource()->SetToCopyDest(commandList);

    m_SecondStatesAreReady = true;
}

void CrossAdapterTextureResources::CopyResource(ComPtr<ID3D12GraphicsCommandList2> commandList, std::shared_ptr<TextureBuffer> src, std::shared_ptr<TextureBuffer> dst, bool toShared)
{
    toShared ? src->SetToCopySource(commandList) : dst->SetToCopyDest(commandList);
    commandList->CopyResource(dst->GetResource().Get(), src->GetResource().Get());
}

void CrossAdapterTextureResources::CopyPrimaryDeviceDataToSharedMemory(TestTime* test)
{
    test->BeginPass();

    ComPtr<ID3D12GraphicsCommandList2> commandList = m_PrimaryCopyCommandQueue->GetCommandList();

    if (!m_PrimaryStatesAreReady)
        SetStatesToPrimaryResources(commandList);

    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::POSITION), m_SharedPositionBuffer->GetPrimarySharedResource(),   true);
    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::NORMAL),   m_SharedNormalBuffer->GetPrimarySharedResource(),     true);
    CopyResource(commandList, PrimaryGBuffer->GetBuffer(GBuffer::ORM),      m_SharedORMBuffer->GetPrimarySharedResource(),        true);
    CopyResource(commandList, PrimaryLightPassResult,                       m_SharedLightPassBuffer->GetPrimarySharedResource(),  true);

    m_CopyPrimaryDeviceDataToSharedMemoryFenceValue = m_PrimaryCopyCommandQueue->ExecuteCommandList(commandList);

    test->EndPass(CurrentPass::CopyPrimaryDeviceDataToSharedMemory);
}

void CrossAdapterTextureResources::WaitForCopyingPrimaryDeviceDataToSharedMemory(TestTime* test)
{
    test->BeginPass();
    m_PrimaryCopyCommandQueue->WaitForFenceValue(m_CopyPrimaryDeviceDataToSharedMemoryFenceValue);
    test->EndPass(CurrentPass::WaitPrimaryDeviceDataToSharedMemory);
}

void CrossAdapterTextureResources::CopySharedMemoryDataToPrimaryDevice(TestTime* test)
{
    test->BeginPass();

    ComPtr<ID3D12GraphicsCommandList2> commandList = m_PrimaryCopyCommandQueue->GetCommandList();

    if (!m_PrimaryStatesAreReady)
        SetStatesToPrimaryResources(commandList);

    CopyResource(commandList, m_SharedSSRBuffer->GetPrimarySharedResource(), PrimarySSRResult, false);

    m_CopySharedMemoryDataToPrimaryDeviceFenceValue = m_PrimaryCopyCommandQueue->ExecuteCommandList(commandList);

    test->EndPass(CurrentPass::CopySharedMemoryDataToPrimaryDevice);
}

void CrossAdapterTextureResources::WaitForCopyingSharedMemoryDataToPrimaryDevice(TestTime* test)
{
    test->BeginPass();
    m_PrimaryCopyCommandQueue->WaitForFenceValue(m_CopySharedMemoryDataToPrimaryDeviceFenceValue);
    test->EndPass(CurrentPass::WaitSharedMemoryDataToPrimaryDevice);
}

void CrossAdapterTextureResources::CopySecondDeviceDataToSharedMemory(TestTime* test)
{
    test->BeginPass();

    ComPtr<ID3D12GraphicsCommandList2> commandList = m_SecondCopyCommandQueue->GetCommandList();

    if (!m_SecondStatesAreReady)
        SetStatesToSharedResources(commandList);

    CopyResource(commandList, SecondSSRResult, m_SharedSSRBuffer->GetSecondSharedResource(), true);

    m_CopySecondDeviceDataToSharedMemoryFenceValue = m_SecondCopyCommandQueue->ExecuteCommandList(commandList);

    test->EndPass(CurrentPass::CopySecondDeviceDataToSharedMemory);
}

void CrossAdapterTextureResources::WaitForCopyingSecondDeviceDataToSharedMemory(TestTime* test)
{
    test->BeginPass();
    m_SecondCopyCommandQueue->WaitForFenceValue(m_CopySecondDeviceDataToSharedMemoryFenceValue);
    test->EndPass(CurrentPass::WaitSecondDeviceDataToSharedMemory);
}

void CrossAdapterTextureResources::CopySharedMemoryDataToSecondDevice(TestTime* test)
{
    test->BeginPass();

    ComPtr<ID3D12GraphicsCommandList2> commandList = m_SecondCopyCommandQueue->GetCommandList();

    if (!m_SecondStatesAreReady)
        SetStatesToSharedResources(commandList);

    CopyResource(commandList, m_SharedPositionBuffer->GetSecondSharedResource(),  SecondGBuffer->GetBuffer(GBuffer::POSITION),  false);
    CopyResource(commandList, m_SharedNormalBuffer->GetSecondSharedResource(),    SecondGBuffer->GetBuffer(GBuffer::NORMAL),    false);
    CopyResource(commandList, m_SharedORMBuffer->GetSecondSharedResource(),       SecondGBuffer->GetBuffer(GBuffer::ORM),       false);
    CopyResource(commandList, m_SharedLightPassBuffer->GetSecondSharedResource(), SecondLightPassResult,                        false);

    m_CopySharedMemoryDataToSecondDeviceFenceValue = m_SecondCopyCommandQueue->ExecuteCommandList(commandList);

    test->EndPass(CurrentPass::CopySharedMemoryDataToSecondDevice);
}

void CrossAdapterTextureResources::WaitForCopyingSharedMemoryDataToSecondDevice(TestTime* test)
{
    test->BeginPass();
    m_SecondCopyCommandQueue->WaitForFenceValue(m_CopySharedMemoryDataToSecondDeviceFenceValue);
    test->EndPass(CurrentPass::WaitSharedMemoryDataToSecondDevice);
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

    m_SharedPositionBuffer->Resize(width, height);
    m_SharedNormalBuffer->Resize(width, height);
    m_SharedORMBuffer->Resize(width, height);
    m_SharedLightPassBuffer->Resize(width, height);
    m_SharedSSRBuffer->Resize(width, height);
}

void CrossAdapterTextureResources::Destroy()
{
    m_SharedPositionBuffer->Destroy();
    m_SharedPositionBuffer.reset();
    m_SharedPositionBuffer = nullptr;

    m_SharedNormalBuffer->Destroy();
    m_SharedNormalBuffer.reset();
    m_SharedNormalBuffer = nullptr;

    m_SharedORMBuffer->Destroy();
    m_SharedORMBuffer.reset();
    m_SharedORMBuffer = nullptr;

    m_SharedLightPassBuffer->Destroy();
    m_SharedLightPassBuffer.reset();
    m_SharedLightPassBuffer = nullptr;

    m_SharedSSRBuffer->Destroy();
    m_SharedSSRBuffer.reset();
    m_SharedSSRBuffer = nullptr;


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


    m_PrimaryDevice.Reset();
    m_PrimaryDevice = nullptr;

    m_SecondDevice.Reset();
    m_SecondDevice = nullptr;
}
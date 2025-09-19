#include "../GBuffer.h"
#include "../../Base/DescriptorHeaps.h"
#include "../ShaderResources.h"

void GBuffer::Init(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter, UINT width, UINT height)
{
    static const DXGI_FORMAT formats[GBUFFER_COUNT] =
    {
        DXGI_FORMAT_R32G32B32A32_FLOAT, // Position
        DXGI_FORMAT_R16G16B16A16_FLOAT, // Normal
        DXGI_FORMAT_R8G8B8A8_UNORM,     // Diffuse
        DXGI_FORMAT_R8G8B8A8_UNORM,     // Emissive
        DXGI_FORMAT_R8G8B8A8_UNORM      // ORM
    };

    LPCWSTR names[GBUFFER_COUNT]
    {
        graphicsAdapter == GraphicAdapterPrimary ? L"GBufferPositionPrimaryResource" : L"GBufferPositionSecondResource",
        graphicsAdapter == GraphicAdapterPrimary ? L"GBufferNormalPrimaryResource"  : L"GBufferNormalSecondResource",
        graphicsAdapter == GraphicAdapterPrimary ? L"GBufferDiffusePrimaryResource" : L"GBufferDiffuseSecondResource",
        graphicsAdapter == GraphicAdapterPrimary ? L"GBufferEmissivePrimaryResource" : L"GBufferEmissiveSecondResource",
        graphicsAdapter == GraphicAdapterPrimary ? L"GBufferORMPrimaryResource"     : L"GBufferORMSecondResource",
    };

    for (int i = 0; i < GBUFFER_COUNT; i++)
    {
        m_Targets[i] = std::make_shared<TextureBuffer>();
        
        m_Targets[i]->SetName(names[i]);
        m_Targets[i]->Init(device, graphicsAdapter, width, height, formats[i]);
    }
}

void GBuffer::Release()
{
    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        m_Targets[i]->Release();
    }
}

void GBuffer::Resize(UINT width, UINT height)
{
    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        m_Targets[i]->Resize(width, height);
    }
}

void GBuffer::BindRenderTargets(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
{
    const D3D12_CPU_DESCRIPTOR_HANDLE rtvs[] =
    {
        m_Targets[0]->RtvCPU(),
        m_Targets[1]->RtvCPU(),
        m_Targets[2]->RtvCPU(),
        m_Targets[3]->RtvCPU(),
        m_Targets[4]->RtvCPU(),
    };

    commandList->OMSetRenderTargets(GBUFFER_COUNT, rtvs, FALSE, &dsvHandle);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE GBuffer::SrvGPU(TargetType type) const
{
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_Targets[type]->SrvGPU());
}

CD3DX12_CPU_DESCRIPTOR_HANDLE GBuffer::RtvCPU(TargetType type) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_Targets[type]->RtvCPU());
}

void GBuffer::SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        m_Targets[i]->SetToWriteAndClear(commandList);
    }
}

void GBuffer::SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    for (size_t i = 0; i < GBUFFER_COUNT; i++)
    {
        m_Targets[i]->SetToRead(commandList);
    }
}

void GBuffer::SetGraphicsRootDescriptorTable(int slot, TargetType type, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    m_Targets[type]->SetGraphicsRootDescriptorTable(slot, commandList);
}

std::shared_ptr<TextureBuffer> GBuffer::GetBuffer(TargetType type)
{
    return m_Targets[type];
}

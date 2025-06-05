#pragma once

#include "../Base/DX12LibPCH.h"

class SSRPipeline
{
public:
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;

    void Initialize(ComPtr<ID3D12Device2> device);
    void Set(ComPtr<ID3D12GraphicsCommandList2> commandList);

protected:
    ComPtr<ID3DBlob> m_VertexShaderBlob;
    ComPtr<ID3DBlob> m_PixelShaderBlob;    

    void CreateRootSignature(ComPtr<ID3D12Device2> device);
    void CreatePipelineState(ComPtr<ID3D12Device2> device);
};
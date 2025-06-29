#pragma once
#include "../Base/DX12LibPCH.h"

using namespace std;

class SimplePipeline
{
private:
    const static int InputLayoutCount = 2;
public:
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;

    void Initialize(ComPtr<ID3D12Device2> device);
    void Set(ComPtr<ID3D12GraphicsCommandList2> commandList);

protected:
    ComPtr<ID3DBlob> m_VertexShaderBlob;
    ComPtr<ID3DBlob> m_PixelShaderBlob;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE m_RootSignatureFeatureData = {};
    D3D12_ROOT_SIGNATURE_FLAGS m_RootSignatureFlags;
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_RootSignatureDescription;
    CD3DX12_RASTERIZER_DESC m_RasterizerDesc;
    ComPtr<ID3DBlob> m_RootSignatureBlob;
    D3D12_INPUT_ELEMENT_DESC m_InputLayout[InputLayoutCount];

    void LoadVertexShader();
    void LoadPixelShader();
    void CreateVertexInputLayout();
    void CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device);
    void CreateRootSignatureFlags();
    void CreateRootSignatureBlob();
    void CreateRootSignature(ComPtr<ID3D12Device2> device);
    void CreateRasterizerDesc();
    void CreatePipelineState(ComPtr<ID3D12Device2> device);
};
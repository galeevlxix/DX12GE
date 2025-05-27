#pragma once
#include "DX12LibPCH.h"

class GeometryPassPipeline
{
private:
    const static int InputLayoutCount = 5;
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

    virtual void LoadVertexShader();
    virtual void LoadPixelShader();
    virtual void CreateVertexInputLayout();
    void CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device);
    void CreateRootSignatureFlags();
    virtual void CreateRootSignatureBlob();
    void CreateRootSignature(ComPtr<ID3D12Device2> device);
    virtual void CreateRasterizerDesc();
    void CreatePipelineState(ComPtr<ID3D12Device2> device);
};
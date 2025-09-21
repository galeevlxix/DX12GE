#pragma once
#include "../Base/DX12LibPCH.h"

using namespace std;

class ParticleComputePipeline
{
public:
    ComPtr<ID3D12RootSignature> RootSignature;

    ComPtr<ID3D12PipelineState> UpdatePipelineState;
    ComPtr<ID3D12PipelineState> SortPipelineState;

    void Initialize(ComPtr<ID3D12Device2> device);
    void SetUpdatePSO(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void SetSortPSO(ComPtr<ID3D12GraphicsCommandList2> commandList);
protected:
    ComPtr<ID3DBlob> m_UpdateShaderBlob;
    ComPtr<ID3DBlob> m_SortShaderBlob;

    D3D12_FEATURE_DATA_ROOT_SIGNATURE m_RootSignatureFeatureData = {};
    D3D12_ROOT_SIGNATURE_FLAGS m_RootSignatureFlags;
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_RootSignatureDescription;
    CD3DX12_RASTERIZER_DESC m_RasterizerDesc;
    ComPtr<ID3DBlob> m_RootSignatureBlob;

    void LoadShaders();
    void CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device);
    void CreateRootSignature(ComPtr<ID3D12Device2> device);
    void CreatePipelineState(ComPtr<ID3D12Device2> device);

public:
    void Destroy()
    {
        RootSignature.Reset();
        RootSignature = nullptr;

        UpdatePipelineState.Reset();
        UpdatePipelineState = nullptr;

        SortPipelineState.Reset();
        SortPipelineState = nullptr;

        m_UpdateShaderBlob.Reset();
        m_UpdateShaderBlob = nullptr;

        m_SortShaderBlob.Reset();
        m_SortShaderBlob = nullptr;

        m_RootSignatureBlob.Reset();
        m_RootSignatureBlob = nullptr;
    }
};
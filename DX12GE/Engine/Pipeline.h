#include "DX12LibPCH.h"

using namespace std;

class Pipeline
{
public:
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;

    void Initialize(ComPtr<ID3D12Device2> device);
private:
    ComPtr<ID3DBlob> m_VertexShaderBlob;
    ComPtr<ID3DBlob> m_PixelShaderBlob;
    D3D12_INPUT_ELEMENT_DESC m_InputLayout[3];
    D3D12_FEATURE_DATA_ROOT_SIGNATURE m_RootSignatureFeatureData = {};
    D3D12_ROOT_SIGNATURE_FLAGS m_RootSignatureFlags;
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_RootSignatureDescription;
    ComPtr<ID3DBlob> m_RootSignatureBlob;

    void LoadVertexShader();
    void LoadPixelShader();
    void CreateVertexInputLayout();
    void CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device);
    void CreateRootSignatureFlags();
    void CreateRootSignatureBlob();
    void CreateRootSignature(ComPtr<ID3D12Device2> device);
    void CreatePipelineState(ComPtr<ID3D12Device2> device);
};
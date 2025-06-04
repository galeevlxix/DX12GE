#include "../TexturedPipeline.h"
#include "DirectXMath.h"

using namespace DirectX;

void TexturedPipeline::LoadVertexShader()
{
	ThrowIfFailed(
		D3DReadFileToBlob(L"TexturedVertexShader.cso", &m_VertexShaderBlob));
}

void TexturedPipeline::LoadPixelShader()
{
	ThrowIfFailed(
		D3DReadFileToBlob(L"TexturedPixelShader.cso", &m_PixelShaderBlob));
}

void TexturedPipeline::CreateVertexInputLayout()
{
	m_InputLayout[0] = { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	m_InputLayout[1] = { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}

void TexturedPipeline::CreateRootSignatureBlob()
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[2];

    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 2, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    const CD3DX12_DESCRIPTOR_RANGE1 descRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[1].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

    m_RootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1, &staticSampler, m_RootSignatureFlags);

    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(
        D3DX12SerializeVersionedRootSignature(&m_RootSignatureDescription, m_RootSignatureFeatureData.HighestVersion, &m_RootSignatureBlob, &errorBlob));
}

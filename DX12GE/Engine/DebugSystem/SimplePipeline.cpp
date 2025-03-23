#include "SimplePipeline.h"
#include "DirectXMath.h"

using namespace DirectX;

void SimplePipeline::Initialize(ComPtr<ID3D12Device2> device)
{
    LoadVertexShader();
    LoadPixelShader();
    CreateVertexInputLayout();
    CreateRootSignatureFeatureData(device);
    CreateRootSignatureFlags();
    CreateRootSignatureBlob();
    CreateRootSignature(device);
    CreateRasterizerDesc();
    CreatePipelineState(device);
}

void SimplePipeline::Set(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetPipelineState(PipelineState.Get());
    commandList->SetGraphicsRootSignature(RootSignature.Get());
}

// Load the vertex shader
void SimplePipeline::LoadVertexShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"SimpleVertexShader.cso", &m_VertexShaderBlob));
}

// Load the pixel shader
void SimplePipeline::LoadPixelShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"SimplePixelShader.cso", &m_PixelShaderBlob));
}

void SimplePipeline::CreateVertexInputLayout()
{
    m_InputLayout[0] = { "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_InputLayout[1] = { "COLOR",       0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}

void SimplePipeline::CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device)
{
    m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &m_RootSignatureFeatureData, sizeof(m_RootSignatureFeatureData))))
    {
        m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
}

// Allow input layout and deny unnecessary access to certain pipeline stages
void SimplePipeline::CreateRootSignatureFlags()
{
    m_RootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
}

// Init root parameters that are used by shaders
void SimplePipeline::CreateRootSignatureBlob()
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    m_RootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, m_RootSignatureFlags);

    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(
        D3DX12SerializeVersionedRootSignature(&m_RootSignatureDescription, m_RootSignatureFeatureData.HighestVersion, &m_RootSignatureBlob, &errorBlob));
}

// Create the root signature
void SimplePipeline::CreateRootSignature(ComPtr<ID3D12Device2> device)
{
    ThrowIfFailed(
        device->CreateRootSignature(0, m_RootSignatureBlob->GetBufferPointer(), m_RootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

void SimplePipeline::CreateRasterizerDesc()
{
    m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    m_RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    m_RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
}

// Create the pipeline state
void SimplePipeline::CreatePipelineState(ComPtr<ID3D12Device2> device)
{
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = RootSignature.Get();
    pipelineStateStream.InputLayout = { m_InputLayout, _countof(m_InputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(m_VertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(m_PixelShaderBlob.Get());
    pipelineStateStream.Rasterizer = m_RasterizerDesc;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
    {
        sizeof(PipelineStateStream), &pipelineStateStream
    };

    ThrowIfFailed(
        device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&PipelineState)));
}
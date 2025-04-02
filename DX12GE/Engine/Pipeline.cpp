#include "Pipeline.h"
#include "DirectXMath.h"
#include "../Game/LightManager.h"

using namespace DirectX;

void Pipeline::Initialize(ComPtr<ID3D12Device2> device)
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

void Pipeline::Set(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetPipelineState(PipelineState.Get());
    commandList->SetGraphicsRootSignature(RootSignature.Get());
}

// Load the vertex shader
void Pipeline::LoadVertexShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"VertexShader.cso", &m_VertexShaderBlob));
}

// Load the pixel shader
void Pipeline::LoadPixelShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"PixelShader.cso", &m_PixelShaderBlob));
}

void Pipeline::CreateVertexInputLayout()
{   
    m_InputLayout[0] = { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_InputLayout[1] = { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_InputLayout[2] = { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}

void Pipeline::CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device)
{
    m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &m_RootSignatureFeatureData, sizeof(m_RootSignatureFeatureData))))
    {
        m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
}

// Allow input layout and deny unnecessary access to certain pipeline stages
void Pipeline::CreateRootSignatureFlags()
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
void Pipeline::CreateRootSignatureBlob()
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[8];

    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 2, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    const CD3DX12_DESCRIPTOR_RANGE1 descRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[1].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[2].InitAsConstants(LightManager::SizeOfAmbientLight() / 4, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[3].InitAsConstants(LightManager::SizeOfDirectionalLight() / 4, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[4].InitAsConstants(LightManager::SizeOfLightProperties() / 4, 2, 0, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[5].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[6].InitAsShaderResourceView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[7].InitAsConstants(LightManager::SizeOfSpecularLight() / 4, 3, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_STATIC_SAMPLER_DESC staticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);
    
    m_RootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1, &staticSampler, m_RootSignatureFlags);

    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(
        D3DX12SerializeVersionedRootSignature(&m_RootSignatureDescription, m_RootSignatureFeatureData.HighestVersion, &m_RootSignatureBlob, &errorBlob));
}

// Create the root signature
void Pipeline::CreateRootSignature(ComPtr<ID3D12Device2> device)
{
    ThrowIfFailed(
        device->CreateRootSignature(0, m_RootSignatureBlob->GetBufferPointer(), m_RootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

void Pipeline::CreateRasterizerDesc()
{
    m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    m_RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    m_RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
}

// Create the pipeline state
void Pipeline::CreatePipelineState(ComPtr<ID3D12Device2> device)
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
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
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

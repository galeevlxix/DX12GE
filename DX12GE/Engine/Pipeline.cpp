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
    CD3DX12_ROOT_PARAMETER1 rootParameters[10];

    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);  //objConst
    rootParameters[1].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);   //worldConst
    
    rootParameters[2].InitAsShaderResourceView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);   //pointLights
    rootParameters[3].InitAsShaderResourceView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);   //spotLights

    const CD3DX12_DESCRIPTOR_RANGE1 diffuseTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
    rootParameters[4].InitAsDescriptorTable(1, &diffuseTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 smDescTable1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
    rootParameters[5].InitAsDescriptorTable(1, &smDescTable1, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 smDescTable2(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
    rootParameters[6].InitAsDescriptorTable(1, &smDescTable2, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 smDescTable3(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
    rootParameters[7].InitAsDescriptorTable(1, &smDescTable3, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 smDescTable4(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);
    rootParameters[8].InitAsDescriptorTable(1, &smDescTable4, D3D12_SHADER_VISIBILITY_PIXEL);

    rootParameters[9].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);  //shadowTrans

    const CD3DX12_STATIC_SAMPLER_DESC samplers[2] =
    {
        // textureSampler
        CD3DX12_STATIC_SAMPLER_DESC (
        0,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        0.0f,
        16,
        D3D12_COMPARISON_FUNC_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE
        ), 

        // shadowSampler
        CD3DX12_STATIC_SAMPLER_DESC (
        1,
        D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
        D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
        0.0f,                               // mipLODBias
        16,                                 // maxAnisotropy
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
    };

    m_RootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 2, samplers, m_RootSignatureFlags);

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

#include "../ParticlePipeline.h"

void ParticlePipeline::Initialize(ComPtr<ID3D12Device2> device)
{
    LoadVertexShader();
    LoadGeometryShader();
    LoadPixelShader();
    CreateVertexInputLayout();
    CreateRootSignatureFeatureData(device);
    CreateRootSignatureFlags();
    CreateRootSignatureBlob();
    CreateRootSignature(device);
    CreateRasterizerDesc();
    CreatePipelineState(device);

    PipelineState.Get()->SetName(L"Draw Particle Pipeline State");
    RootSignature.Get()->SetName(L"Draw Particle Root Signature");
}

void ParticlePipeline::Set(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetPipelineState(PipelineState.Get());
    commandList->SetGraphicsRootSignature(RootSignature.Get());
}

void ParticlePipeline::LoadVertexShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"ParticleVertexShader.cso", &m_VertexShaderBlob));
}

void ParticlePipeline::LoadGeometryShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"ParticleGeometryShader.cso", &m_GeometryShaderBlob));
}

void ParticlePipeline::LoadPixelShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"ParticlePixelShader.cso", &m_PixelShaderBlob));
}

void ParticlePipeline::CreateVertexInputLayout()
{
    m_InputLayout[0] = { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_InputLayout[1] = { "VELOCITY",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}

void ParticlePipeline::CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device)
{
    m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &m_RootSignatureFeatureData, sizeof(m_RootSignatureFeatureData))))
    {
        m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
}

void ParticlePipeline::CreateRootSignatureFlags()
{
    m_RootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
}

void ParticlePipeline::CreateRootSignatureBlob()
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[3];

    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_GEOMETRY);  //particle const buffer
    
    const CD3DX12_DESCRIPTOR_RANGE1 diffuseTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[1].InitAsDescriptorTable(1, &diffuseTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);

    rootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);  //world const buffer

    const CD3DX12_STATIC_SAMPLER_DESC samplers[1] =
    {
        // textureSampler
        CD3DX12_STATIC_SAMPLER_DESC(
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
    };

    m_RootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1, samplers, m_RootSignatureFlags);

    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(
        D3DX12SerializeVersionedRootSignature(&m_RootSignatureDescription, m_RootSignatureFeatureData.HighestVersion, &m_RootSignatureBlob, &errorBlob));
}

void ParticlePipeline::CreateRootSignature(ComPtr<ID3D12Device2> device)
{
    ThrowIfFailed(
        device->CreateRootSignature(0, m_RootSignatureBlob->GetBufferPointer(), m_RootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

void ParticlePipeline::CreateRasterizerDesc()
{
    m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    m_RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    m_RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
}

void ParticlePipeline::CreatePipelineState(ComPtr<ID3D12Device2> device)
{
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_GS GS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendDesc;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;       // Diffuse

    pipelineStateStream.pRootSignature = RootSignature.Get();
    pipelineStateStream.InputLayout = { m_InputLayout, _countof(m_InputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(m_VertexShaderBlob.Get());
    pipelineStateStream.GS = CD3DX12_SHADER_BYTECODE(m_GeometryShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(m_PixelShaderBlob.Get());
    pipelineStateStream.Rasterizer = m_RasterizerDesc;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    CD3DX12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    pipelineStateStream.BlendDesc = blendDesc;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
    {
        sizeof(PipelineStateStream), &pipelineStateStream
    };

    ThrowIfFailed(
        device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&PipelineState)));
}

#include "../GeometryPassPipeline.h"

void GeometryPassPipeline::Initialize(ComPtr<ID3D12Device2> device)
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

    PipelineState.Get()->SetName(L"Geometry Pass Pipeline State");
    RootSignature.Get()->SetName(L"Geometry Pass Root Signature");
}

void GeometryPassPipeline::Set(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetPipelineState(PipelineState.Get());
    commandList->SetGraphicsRootSignature(RootSignature.Get());
}

void GeometryPassPipeline::LoadVertexShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"GeometryPassVertexShader.cso", &m_VertexShaderBlob));
}

void GeometryPassPipeline::LoadPixelShader()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"GeometryPassPixelShader.cso", &m_PixelShaderBlob));
}

void GeometryPassPipeline::CreateVertexInputLayout()
{
    m_InputLayout[0] = { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_InputLayout[1] = { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_InputLayout[2] = { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_InputLayout[3] = { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_InputLayout[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
}

void GeometryPassPipeline::CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device)
{
    m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &m_RootSignatureFeatureData, sizeof(m_RootSignatureFeatureData))))
    {
        m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
}

void GeometryPassPipeline::CreateRootSignatureFlags()
{
    m_RootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;
}

void GeometryPassPipeline::CreateRootSignatureBlob()
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[9];

    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX);  //objConst
    rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);  //matConst
    
    const CD3DX12_DESCRIPTOR_RANGE1 diffuseTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[2].InitAsDescriptorTable(1, &diffuseTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 normalTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
    rootParameters[3].InitAsDescriptorTable(1, &normalTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 emissiveTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
    rootParameters[4].InitAsDescriptorTable(1, &emissiveTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 occlusionTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
    rootParameters[5].InitAsDescriptorTable(1, &occlusionTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 roughnessTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
    rootParameters[6].InitAsDescriptorTable(1, &roughnessTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 metallicTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5);
    rootParameters[7].InitAsDescriptorTable(1, &metallicTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_DESCRIPTOR_RANGE1 combinedTexDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6);
    rootParameters[8].InitAsDescriptorTable(1, &combinedTexDescRange, D3D12_SHADER_VISIBILITY_PIXEL);
    
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

void GeometryPassPipeline::CreateRootSignature(ComPtr<ID3D12Device2> device)
{
    ThrowIfFailed(
        device->CreateRootSignature(0, m_RootSignatureBlob->GetBufferPointer(), m_RootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

void GeometryPassPipeline::CreateRasterizerDesc()
{
    m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    m_RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    m_RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
}

void GeometryPassPipeline::CreatePipelineState(ComPtr<ID3D12Device2> device)
{
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER Rasterizer;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilDesc;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 5;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;   // Position
    rtvFormats.RTFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;   // Normal
    rtvFormats.RTFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;       // Diffuse
    rtvFormats.RTFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;       // Emissive
    rtvFormats.RTFormats[4] = DXGI_FORMAT_R8G8B8A8_UNORM;       // Orm

    pipelineStateStream.pRootSignature = RootSignature.Get();
    pipelineStateStream.InputLayout = { m_InputLayout, _countof(m_InputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(m_VertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(m_PixelShaderBlob.Get());
    pipelineStateStream.Rasterizer = m_RasterizerDesc;
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;                             
    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;    
    dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    dsDesc.StencilEnable = FALSE;
    dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    pipelineStateStream.DepthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(dsDesc);

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
    {
        sizeof(PipelineStateStream), &pipelineStateStream
    };

    ThrowIfFailed(
        device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&PipelineState)));
}

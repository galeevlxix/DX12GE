#include "../SSRPipeline.h"

void SSRPipeline::Initialize(ComPtr<ID3D12Device2> device)
{
	CreateRootSignature(device);
	CreatePipelineState(device);

	PipelineState.Get()->SetName(L"SSR Pipeline State");
	RootSignature.Get()->SetName(L"SSR Root Signature");
}

void SSRPipeline::Set(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	commandList->SetPipelineState(PipelineState.Get());
	commandList->SetGraphicsRootSignature(RootSignature.Get());
}

void SSRPipeline::CreateRootSignature(ComPtr<ID3D12Device2> device)
{
	ThrowIfFailed(
		D3DReadFileToBlob(L"SSRVertexShader.cso", &m_VertexShaderBlob));
	ThrowIfFailed(
		D3DReadFileToBlob(L"SSRPixelShader.cso", &m_PixelShaderBlob));

	D3D12_FEATURE_DATA_ROOT_SIGNATURE m_RootSignatureFeatureData = {};
	m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &m_RootSignatureFeatureData, sizeof(m_RootSignatureFeatureData))))
	{
		m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	D3D12_ROOT_SIGNATURE_FLAGS m_RootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	CD3DX12_ROOT_PARAMETER1 rootParameters[6];

	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_PIXEL);   // ssr const buffer

	const CD3DX12_DESCRIPTOR_RANGE1 posTexDesc(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	rootParameters[1].InitAsDescriptorTable(1, &posTexDesc, D3D12_SHADER_VISIBILITY_PIXEL);

	const CD3DX12_DESCRIPTOR_RANGE1 normTexDesc(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
	rootParameters[2].InitAsDescriptorTable(1, &normTexDesc, D3D12_SHADER_VISIBILITY_PIXEL);

	const CD3DX12_DESCRIPTOR_RANGE1 ormTexDesc(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2);
	rootParameters[3].InitAsDescriptorTable(1, &ormTexDesc, D3D12_SHADER_VISIBILITY_PIXEL);

	const CD3DX12_DESCRIPTOR_RANGE1 colTexDesc(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3);
	rootParameters[4].InitAsDescriptorTable(1, &colTexDesc, D3D12_SHADER_VISIBILITY_PIXEL);

	const CD3DX12_DESCRIPTOR_RANGE1 skyTexDesc(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
	rootParameters[5].InitAsDescriptorTable(1, &skyTexDesc, D3D12_SHADER_VISIBILITY_PIXEL);

    const CD3DX12_STATIC_SAMPLER_DESC samplers[1] =
    {
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
        )
    };

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_RootSignatureDescription;
	m_RootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, _countof(samplers), samplers, m_RootSignatureFlags);

	ComPtr<ID3DBlob> errorBlob;
	ComPtr<ID3DBlob> m_RootSignatureBlob;
	ThrowIfFailed(
		D3DX12SerializeVersionedRootSignature(&m_RootSignatureDescription, m_RootSignatureFeatureData.HighestVersion, &m_RootSignatureBlob, &errorBlob));

	ThrowIfFailed(
		device->CreateRootSignature(0, m_RootSignatureBlob->GetBufferPointer(), m_RootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

void SSRPipeline::CreatePipelineState(ComPtr<ID3D12Device2> device)
{
	CD3DX12_RASTERIZER_DESC m_RasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	m_RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	m_RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

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
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D12_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;                              // тест глубины включён
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;    // но не пишем в Z
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	pipelineStateStream.pRootSignature = RootSignature.Get();
	pipelineStateStream.InputLayout = { nullptr, 0 };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(m_VertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(m_PixelShaderBlob.Get());
	pipelineStateStream.Rasterizer = m_RasterizerDesc;
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;
	pipelineStateStream.DepthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(dsDesc);

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
	{
		sizeof(PipelineStateStream), &pipelineStateStream
	};

	ThrowIfFailed(
		device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&PipelineState)));
}

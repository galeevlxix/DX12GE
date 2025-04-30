#include "ParticleComputePipeline.h"

void ParticleComputePipeline::Initialize(ComPtr<ID3D12Device2> device)
{
    LoadShaders();
    CreateRootSignatureFeatureData(device);
    CreateRootSignature(device);
    CreatePipelineState(device);
}

void ParticleComputePipeline::SetUpdatePSO(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetPipelineState(UpdatePipelineState.Get());
    commandList->SetComputeRootSignature(RootSignature.Get());
}

void ParticleComputePipeline::SetSortPSO(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetPipelineState(SortPipelineState.Get());
    commandList->SetComputeRootSignature(RootSignature.Get());
}

void ParticleComputePipeline::LoadShaders()
{
    ThrowIfFailed(
        D3DReadFileToBlob(L"ParticleComputeShader.cso", &m_UpdateShaderBlob));
    ThrowIfFailed(
        D3DReadFileToBlob(L"SortParticleComputeShader.cso", &m_SortShaderBlob));
}

void ParticleComputePipeline::CreateRootSignatureFeatureData(ComPtr<ID3D12Device2> device)
{
    m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &m_RootSignatureFeatureData, sizeof(m_RootSignatureFeatureData))))
    {
        m_RootSignatureFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
}

void ParticleComputePipeline::CreateRootSignature(ComPtr<ID3D12Device2> device)
{
    CD3DX12_ROOT_PARAMETER1 rootParameters[3];
    
    // b0
    rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);

    // u0
    const CD3DX12_DESCRIPTOR_RANGE1 particleBufferRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
    rootParameters[1].InitAsDescriptorTable(1, &particleBufferRange, D3D12_SHADER_VISIBILITY_ALL);

    // t0
    const CD3DX12_DESCRIPTOR_RANGE1 tex3DescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    rootParameters[2].InitAsDescriptorTable(1, &tex3DescRange, D3D12_SHADER_VISIBILITY_ALL);

    // s0
    CD3DX12_STATIC_SAMPLER_DESC staticSampler(
        0, 
        D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );

    m_RootSignatureDescription = {};
    m_RootSignatureDescription.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    m_RootSignatureDescription.Desc_1_1.NumParameters = _countof(rootParameters);
    m_RootSignatureDescription.Desc_1_1.pParameters = rootParameters;
    m_RootSignatureDescription.Desc_1_1.NumStaticSamplers = 1;
    m_RootSignatureDescription.Desc_1_1.pStaticSamplers = &staticSampler;
    m_RootSignatureDescription.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(
        D3DX12SerializeVersionedRootSignature(&m_RootSignatureDescription, m_RootSignatureFeatureData.HighestVersion, &m_RootSignatureBlob, &errorBlob));

    ThrowIfFailed(
        device->CreateRootSignature(0, m_RootSignatureBlob->GetBufferPointer(), m_RootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));
}

void ParticleComputePipeline::CreatePipelineState(ComPtr<ID3D12Device2> device)
{
    struct ComputePipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_CS CS;
    } pipelineStateStream;

    pipelineStateStream.pRootSignature = RootSignature.Get();
    pipelineStateStream.CS = CD3DX12_SHADER_BYTECODE(m_UpdateShaderBlob.Get());

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
    {
        sizeof(ComputePipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(
        device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&UpdatePipelineState)));

    pipelineStateStream.CS = CD3DX12_SHADER_BYTECODE(m_SortShaderBlob.Get());
    pipelineStateStreamDesc =
    {
        sizeof(ComputePipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(
        device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&SortPipelineState)));
}

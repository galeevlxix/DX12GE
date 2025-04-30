#include "ShaderResources.h"

static ObjectConstantBuffer* ObjectCB = nullptr;
static ShadowConstantBuffer* ShadowCB = nullptr;
static WorldConstantBuffer* WorldCB = nullptr;
static ParticleConstantBuffer* ParticleCB = nullptr;
static ParticleComputeConstantBuffer* ComputeConstantCB = nullptr;
static BitonicSortConstantBuffer* BitonicCB = nullptr;
static UploadBuffer* mUploadBuffer = nullptr;

static UploadBuffer::Allocation OcbAllocation;
static UploadBuffer::Allocation WcbAllocation;
static UploadBuffer::Allocation ScbAllocation;
static UploadBuffer::Allocation PcbAllocation;
static UploadBuffer::Allocation CcbAllocation;
static UploadBuffer::Allocation BcbAllocation;

static const UINT OcbSize = sizeof(ObjectConstantBuffer);
static const UINT ScbSize = sizeof(ShadowConstantBuffer);
static const UINT WcbSize = sizeof(WorldConstantBuffer);
static const UINT PcbSize = sizeof(ParticleConstantBuffer);
static const UINT CcbSize = sizeof(ParticleComputeConstantBuffer);
static const UINT BcbSize = sizeof(BitonicSortConstantBuffer);

void ShaderResources::Create()
{
	if (!mUploadBuffer)
	{
		mUploadBuffer = new UploadBuffer();
	}

	if (!ObjectCB)
	{
		ObjectCB = new ObjectConstantBuffer();
	}

	if (!ShadowCB)
	{
		ShadowCB = new ShadowConstantBuffer();
	}

	if (!WorldCB)
	{
		WorldCB = new WorldConstantBuffer();
	}

	if (!ParticleCB)
	{
		ParticleCB = new ParticleConstantBuffer();
	}

	if (!ComputeConstantCB)
	{
		ComputeConstantCB = new ParticleComputeConstantBuffer();
	}

	if (!BitonicCB)
	{
		BitonicCB = new BitonicSortConstantBuffer();
	}
}

ObjectConstantBuffer* ShaderResources::GetObjectCB()
{
	return ObjectCB;
}

ShadowConstantBuffer* ShaderResources::GetShadowCB()
{
	return ShadowCB;
}

WorldConstantBuffer* ShaderResources::GetWorldCB()
{
	return WorldCB;
}

ParticleConstantBuffer* ShaderResources::GetParticleCB()
{
	return ParticleCB;
}

ParticleComputeConstantBuffer* ShaderResources::GetParticleComputeCB()
{
	return ComputeConstantCB;
}

BitonicSortConstantBuffer* ShaderResources::GetBitonicSortCB()
{
	return BitonicCB;
}

UploadBuffer* ShaderResources::GetUploadBuffer()
{
	return mUploadBuffer;
}

void ShaderResources::OnDelete()
{
	mUploadBuffer->Reset();
	mUploadBuffer = nullptr;
}

// поменять на автоматическое определение размера

void ShaderResources::SetGraphicsObjectCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	OcbAllocation = mUploadBuffer->Allocate(OcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(OcbAllocation.CPU, ObjectCB, OcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, OcbAllocation.GPU);
}

void ShaderResources::SetGraphicsShadowCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	ScbAllocation = mUploadBuffer->Allocate(ScbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(ScbAllocation.CPU, ShadowCB, ScbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, ScbAllocation.GPU);
}

void ShaderResources::SetGraphicsWorldCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	WcbAllocation = mUploadBuffer->Allocate(WcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(WcbAllocation.CPU, WorldCB, WcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, WcbAllocation.GPU);
}

void ShaderResources::SetGraphicsParticleCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	PcbAllocation = mUploadBuffer->Allocate(PcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(PcbAllocation.CPU, ParticleCB, PcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, PcbAllocation.GPU);
}

void ShaderResources::SetParticleComputeCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	CcbAllocation = mUploadBuffer->Allocate(CcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(CcbAllocation.CPU, ComputeConstantCB, CcbSize);
	commandList->SetComputeRootConstantBufferView(slot, CcbAllocation.GPU);
}

void ShaderResources::SetBitonicSortCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	BcbAllocation = mUploadBuffer->Allocate(BcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(BcbAllocation.CPU, BitonicCB, BcbSize);
	commandList->SetComputeRootConstantBufferView(slot, BcbAllocation.GPU);
}


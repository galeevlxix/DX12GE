#include "ShaderResources.h"

static ObjectConstantBuffer* ObjectCB = nullptr;
static ShadowConstantBuffer* ShadowCB = nullptr;
static WorldConstantBuffer* WorldCB = nullptr;
static UploadBuffer* mUploadBuffer = nullptr;

static UploadBuffer::Allocation OcbAllocation;
static UploadBuffer::Allocation WcbAllocation;
static UploadBuffer::Allocation ScbAllocation;

static const UINT OcbSize = sizeof(ObjectConstantBuffer);
static const UINT ScbSize = sizeof(ShadowConstantBuffer);
static const UINT WcbSize = sizeof(WorldConstantBuffer);

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
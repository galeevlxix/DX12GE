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

void ShaderResourceBuffers::Create()
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

ObjectConstantBuffer* ShaderResourceBuffers::GetObjectCB()
{
	return ObjectCB;
}

ShadowConstantBuffer* ShaderResourceBuffers::GetShadowCB()
{
	return ShadowCB;
}

WorldConstantBuffer* ShaderResourceBuffers::GetWorldCB()
{
	return WorldCB;
}

UploadBuffer* ShaderResourceBuffers::GetUploadBuffer()
{
	return mUploadBuffer;
}

void ShaderResourceBuffers::OnDelete()
{
	mUploadBuffer->Reset();
	mUploadBuffer = nullptr;
}

// поменять на автоматическое определение размера

void ShaderResourceBuffers::SetGraphicsObjectCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	OcbAllocation = mUploadBuffer->Allocate(OcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(OcbAllocation.CPU, ObjectCB, OcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, OcbAllocation.GPU);
}

void ShaderResourceBuffers::SetGraphicsShadowCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	ScbAllocation = mUploadBuffer->Allocate(ScbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(ScbAllocation.CPU, ShadowCB, ScbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, ScbAllocation.GPU);
}

void ShaderResourceBuffers::SetGraphicsWorldCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	WcbAllocation = mUploadBuffer->Allocate(WcbSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(WcbAllocation.CPU, WorldCB, WcbSize);
	commandList->SetGraphicsRootConstantBufferView(slot, WcbAllocation.GPU);
}

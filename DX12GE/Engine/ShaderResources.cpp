#include "ShaderResources.h"

static ObjectConstantBuffer* ObjectCB = nullptr;
static ShadowConstantBuffer* ShadowCB = nullptr;
static WorldConstantBuffer* WorldCB = nullptr;
static UploadBuffer* mUploadBuffer = nullptr;

void ShaderResourceBuffers::Create()
{
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

	if (!mUploadBuffer)
	{
		mUploadBuffer = new UploadBuffer();
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

// поменять на автоматическое определение размера

void ShaderResourceBuffers::SetGraphicsObjectCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	const UINT sizeInBytes = sizeof(ObjectConstantBuffer);
	auto heapAllocation = ShaderResourceBuffers::GetUploadBuffer()->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(heapAllocation.CPU, ObjectCB, sizeInBytes);
	commandList->SetGraphicsRootConstantBufferView(slot, heapAllocation.GPU);
}

void ShaderResourceBuffers::SetGraphicsShadowCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	const UINT sizeInBytes = sizeof(ShadowConstantBuffer);
	auto heapAllocation = ShaderResourceBuffers::GetUploadBuffer()->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(heapAllocation.CPU, ShadowCB, sizeInBytes);
	commandList->SetGraphicsRootConstantBufferView(slot, heapAllocation.GPU);
}

void ShaderResourceBuffers::SetGraphicsWorldCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot)
{
	const UINT sizeInBytes = sizeof(WorldConstantBuffer);
	auto heapAllocation = ShaderResourceBuffers::GetUploadBuffer()->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
	memcpy(heapAllocation.CPU, WorldCB, sizeInBytes);
	commandList->SetGraphicsRootConstantBufferView(slot, heapAllocation.GPU);
}

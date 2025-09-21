#include "../SharedMemory.h"
#include "../ShaderResources.h"

SharedMemory::SharedMemory()
{
	
}

void SharedMemory::CopyToSharedMemory(SharedMemoryTextureBuffer textureType, std::shared_ptr<TextureBuffer> src, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	auto& sharedBuffer = m_SharedBuffers[textureType];
	if (!sharedBuffer.initialized)
	{
		auto device = src->GetDevice();
		auto desc = src->GetResource()->GetDesc();

		device->GetCopyableFootprints(&desc, 0, 1, 0, &sharedBuffer.footprint, &sharedBuffer.numRows, &sharedBuffer.rowSize, &sharedBuffer.totalBytes);

		D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		D3D12_RESOURCE_DESC bufDesc = CD3DX12_RESOURCE_DESC::Buffer(sharedBuffer.totalBytes);
		device->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &bufDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&sharedBuffer.readbackBuffer));
		sharedBuffer.initialized = true;
	}

	// texture -> readback
	auto state = src->GetResourceState();
	src->SetToCopySource(commandList);
	D3D12_TEXTURE_COPY_LOCATION dst{ sharedBuffer.readbackBuffer.Get(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, sharedBuffer.footprint };
	D3D12_TEXTURE_COPY_LOCATION srcLoc{ src->GetResource().Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };
	commandList->CopyTextureRegion(&dst, 0, 0, 0, &srcLoc, nullptr);
	src->SetToState(commandList, state);
}

void SharedMemory::CopyFromSharedMemory(SharedMemoryTextureBuffer textureType, std::shared_ptr<TextureBuffer> dst, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	auto& sharedBuffer = m_SharedBuffers[textureType];

	if (!sharedBuffer.initialized) return;

	if (!sharedBuffer.uploadBuffer)
	{
		auto device = dst->GetDevice();

		D3D12_HEAP_PROPERTIES hpUp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC bufDesc = CD3DX12_RESOURCE_DESC::Buffer(sharedBuffer.totalBytes);
		device->CreateCommittedResource(&hpUp, D3D12_HEAP_FLAG_NONE,
			&bufDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&sharedBuffer.uploadBuffer));
	}

	// readback -> upload
	BYTE* srcPtr = nullptr;
	sharedBuffer.readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&srcPtr));
	BYTE* dstPtr = nullptr;
	sharedBuffer.uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&dstPtr));
	memcpy(dstPtr, srcPtr, sharedBuffer.totalBytes);

	sharedBuffer.uploadBuffer->Unmap(0, nullptr);
	sharedBuffer.readbackBuffer->Unmap(0, nullptr);

	// upload -> texture
	auto state = dst->GetResourceState();
	dst->SetToCopyDest(commandList);
	auto texDst = dst->GetResource();
	D3D12_TEXTURE_COPY_LOCATION srcLoc{ sharedBuffer.uploadBuffer.Get(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, sharedBuffer.footprint };
	D3D12_TEXTURE_COPY_LOCATION dstLoc{ texDst.Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, 0 };
	commandList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);
	dst->SetToState(commandList, state);
}

bool SharedMemory::IsBufferEmpty(SharedMemoryTextureBuffer textureType)
{
	return !m_SharedBuffers[textureType].initialized;
}

void SharedMemory::Destroy()
{
	for (SharedDesc buffer : m_SharedBuffers)
	{
		buffer.Destroy();
	}
}
#pragma once
#include "../Base/DX12LibPCH.h"
#include "TextureBuffer.h"
#include <vector>

enum SharedMemoryTextureBuffer : UINT
{
	SharedMemoryPositionBuffer = 0,
	SharedMemoryNormalBuffer = 1,
	SharedMemoryORMBuffer = 2,
	SharedMemoryLightPassBuffer = 3,
	SharedMemorySSRBuffer = 4,
	SharedMemoryCount = 5,
};

class SharedMemory
{
public:
	SharedMemory();
	void CopyToSharedMemory(SharedMemoryTextureBuffer textureType, std::shared_ptr<TextureBuffer> src, ComPtr<ID3D12GraphicsCommandList2> srcCommandList);
	void CopyFromSharedMemory(SharedMemoryTextureBuffer textureType, std::shared_ptr<TextureBuffer> dst, ComPtr<ID3D12GraphicsCommandList2> dstCommandList);
	bool IsBufferEmpty(SharedMemoryTextureBuffer textureType);

private:
	struct SharedDesc 
	{
		ComPtr<ID3D12Resource> readbackBuffer, uploadBuffer;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		UINT numRows;
		UINT64 rowSize, totalBytes;
		bool initialized = false;
	};

	SharedDesc m_SharedBuffers[SharedMemoryCount];
};
#pragma once

#include "GBuffer.h"
#include "DepthBuffer.h"
#include "CrossAdapterResource.h"	
#include "../Base/TestTime.h"

class CrossAdapterTextureResources
{
	std::shared_ptr<CrossAdapterResource> PositionBuffer;
	std::shared_ptr<CrossAdapterResource> NormalBuffer;
	std::shared_ptr<CrossAdapterResource> ORMBuffer;
	std::shared_ptr<CrossAdapterResource> LightPassBuffer;
	std::shared_ptr<CrossAdapterResource> SSRBuffer;

	std::shared_ptr<CrossAdapterResource> CreateCrossResource(std::shared_ptr<TextureBuffer> buffer, const std::wstring& name);
	void CopyResource(ComPtr<ID3D12GraphicsCommandList2> commandList, std::shared_ptr<TextureBuffer> src, std::shared_ptr<TextureBuffer> dst, bool toShared);

	bool primaryStates = false;
	bool secondStates = false;

public:

	ComPtr<ID3D12Device2>			PrimaryDevice;
	ComPtr<ID3D12Device2>			SecondDevice;

	std::shared_ptr<DepthBuffer>	PrimaryDepthBuffer;
	std::shared_ptr<DepthBuffer>	SecondDepthBuffer;

	std::shared_ptr<GBuffer>		PrimaryGBuffer;
	std::shared_ptr<TextureBuffer>	PrimaryLightPassResult;
	std::shared_ptr<TextureBuffer>	PrimarySSRResult;

	std::shared_ptr<GBuffer>		SecondGBuffer;
	std::shared_ptr<TextureBuffer>	SecondLightPassResult;
	std::shared_ptr<TextureBuffer>	SecondSSRResult;

	void Initialize();
	void SetStatesToPrimaryResources(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetStatesToSharedResources(ComPtr<ID3D12GraphicsCommandList2> commandList);

	// Primary GBuffer -> SharedMemory
	void CopyPrimaryDeviceDataToSharedMemory(TestTime* test);
	// SharedMemory -> Primary SSR
	void CopySharedMemoryDataToPrimaryDevice(TestTime* test);
	// Secondary SSR -> SharedMemory
	void CopySecondDeviceDataToSharedMemory(TestTime* test);
	// SharedMemory -> Secondary GBuffer
	void CopySharedMemoryDataToSecondDevice(TestTime* test);

	void Resize(UINT width, UINT height);

	void Destroy();
};
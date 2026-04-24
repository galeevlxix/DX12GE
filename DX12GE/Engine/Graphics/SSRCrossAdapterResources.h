#pragma once

#include "GBuffer.h"
#include "DepthBuffer.h"
#include "CrossAdapterResource.h"	
#include "../Base/TestTime.h"

class CrossAdapterTextureResources
{
	std::shared_ptr<CrossAdapterResource> m_SharedPositionBuffer;
	std::shared_ptr<CrossAdapterResource> m_SharedNormalBuffer;
	std::shared_ptr<CrossAdapterResource> m_SharedORMBuffer;
	std::shared_ptr<CrossAdapterResource> m_SharedLightPassBuffer;
	std::shared_ptr<CrossAdapterResource> m_SharedSSRBuffer;

	bool m_PrimaryStatesAreReady = false;
	bool m_SecondStatesAreReady = false;

	ComPtr<ID3D12Device2>			m_PrimaryDevice;
	ComPtr<ID3D12Device2>			m_SecondDevice;

	std::shared_ptr<CommandQueue>	m_PrimaryCopyCommandQueue;
	std::shared_ptr<CommandQueue>	m_SecondCopyCommandQueue;

	uint64_t m_CopyPrimaryDeviceDataToSharedMemoryFenceValue;
	uint64_t m_CopySharedMemoryDataToPrimaryDeviceFenceValue;
	uint64_t m_CopySecondDeviceDataToSharedMemoryFenceValue;
	uint64_t m_CopySharedMemoryDataToSecondDeviceFenceValue;

public:

	std::shared_ptr<DepthBuffer>	PrimaryDepthBuffer;
	std::shared_ptr<DepthBuffer>	SecondDepthBuffer;

	std::shared_ptr<GBuffer>		PrimaryGBuffer;
	std::shared_ptr<TextureBuffer>	PrimaryLightPassResult;
	std::shared_ptr<TextureBuffer>	PrimarySSRResult;

	std::shared_ptr<GBuffer>		SecondGBuffer;
	std::shared_ptr<TextureBuffer>	SecondLightPassResult;
	std::shared_ptr<TextureBuffer>	SecondSSRResult;

private:
	std::shared_ptr<CrossAdapterResource> CreateCrossResource(std::shared_ptr<TextureBuffer> buffer, const std::wstring& name);
	void CopyResource(ComPtr<ID3D12GraphicsCommandList2> commandList, std::shared_ptr<TextureBuffer> src, std::shared_ptr<TextureBuffer> dst, bool toShared);

public:

	void Initialize();
	void SetStatesToPrimaryResources(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetStatesToSharedResources(ComPtr<ID3D12GraphicsCommandList2> commandList);

	// Primary GBuffer and LightPass result -> SharedMemory
	void CopyPrimaryDeviceDataToSharedMemory(TestTime* test);
	void WaitForCopyingPrimaryDeviceDataToSharedMemory(TestTime* test);

	// SharedMemory -> Primary SSR result
	void CopySharedMemoryDataToPrimaryDevice(TestTime* test);
	void WaitForCopyingSharedMemoryDataToPrimaryDevice(TestTime* test);

	// Secondary SSR result -> SharedMemory
	void CopySecondDeviceDataToSharedMemory(TestTime* test);
	void WaitForCopyingSecondDeviceDataToSharedMemory(TestTime* test);

	// SharedMemory -> Secondary GBuffer and LightPass result
	void CopySharedMemoryDataToSecondDevice(TestTime* test);
	void WaitForCopyingSharedMemoryDataToSecondDevice(TestTime* test);

	void Resize(UINT width, UINT height);
	void Destroy();
};
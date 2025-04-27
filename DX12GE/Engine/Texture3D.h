#pragma once

#include "DX12LibPCH.h"
#include <string>

class Texture3D
{
private:
	ComPtr<ID3D12Resource> m_Resource;
	ComPtr<ID3D12Resource> m_UploadBuffer;
	int m_SRVHeapIndex;

public:
	void Load(ComPtr<ID3D12GraphicsCommandList2> commandList, int width, int height, int depth);
	void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
};
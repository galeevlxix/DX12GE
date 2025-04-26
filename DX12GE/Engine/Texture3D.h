#pragma once

#include "DX12LibPCH.h"
#include <string>

class Texture3D
{
public:
	ComPtr<ID3D12Resource> m_Resource;
	int m_SRVHeapIndex;

private:
	ComPtr<ID3D12Resource> m_UploadBuffer;

public:
	void Load(ComPtr<ID3D12GraphicsCommandList2> commandList, int width, int height, int depth);
	void CreateShaderResourceView(D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc);
	void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
};
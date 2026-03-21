#pragma once
#include "../Base/DX12LibPCH.h"

constexpr UINT MAXDESCRIPTORS = 1024; //https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support#invariable-limits

struct HandlePair 
{
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

class DescriptorHeaps
{
	UINT m_usedSize[4];							//Used size of each descriptor heap
	UINT m_incrementSize[4];

	ComPtr<ID3D12DescriptorHeap> m_cbvHeap;		//CBV, SRV, UAV
	ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	static void LoadCBV(ComPtr<ID3D12Device2> device, DescriptorHeaps* heaps);
	static void LoadSampler(ComPtr<ID3D12Device2> device, DescriptorHeaps* heaps);
	static void LoadRTV(ComPtr<ID3D12Device2> device, DescriptorHeaps* heaps);
	static void LoadDSV(ComPtr<ID3D12Device2> device, DescriptorHeaps* heaps);

	static DescriptorHeaps* GetHeaps(GraphicsAdapter graphicsAdapter);

public:
	void Destroy();

	static void OnInit(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter);
	static void DestroyAll();

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index, GraphicsAdapter graphicsAdapter);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index, GraphicsAdapter graphicsAdapter);

	static ComPtr<ID3D12DescriptorHeap> GetCBVHeap(GraphicsAdapter graphicsAdapter);
	static ComPtr<ID3D12DescriptorHeap> GetDSVHeap(GraphicsAdapter graphicsAdapter);

	static HandlePair BatchHandles(D3D12_DESCRIPTOR_HEAP_TYPE type, GraphicsAdapter graphicsAdapter);

	static int GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE type, GraphicsAdapter graphicsAdapter);
private:
	static int GetNextFreeCBVIndex(GraphicsAdapter graphicsAdapter);
	static int GetNextFreeRTVIndex(GraphicsAdapter graphicsAdapter);
	static int GetNextFreeDSVIndex(GraphicsAdapter graphicsAdapter);
};
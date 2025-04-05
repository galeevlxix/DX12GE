#pragma once
#include "DX12LibPCH.h"

constexpr UINT MAXDESCRIPTORS = 1024; //https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support#invariable-limits

struct HandlePair {
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

class DescriptorHeaps 
{
	static UINT m_usedSize[4]; //Used size of each descriptor heap
	static UINT m_incrementSize[4];

	static ComPtr<ID3D12DescriptorHeap> m_cbvHeap; //CBV, SRV, UAV
	static ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
	static ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	static ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	static void LoadCBV(ID3D12Device* device);
	static void LoadSampler(ID3D12Device* device);
	static void LoadRTV(ID3D12Device* device);
	static void LoadDSV(ID3D12Device* device);
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index);
public:
	static void OnInit(ID3D12Device* device);

	static ID3D12DescriptorHeap* GetCBVHeap(); //For binding to command list

	static HandlePair BatchHandles(D3D12_DESCRIPTOR_HEAP_TYPE type);
};
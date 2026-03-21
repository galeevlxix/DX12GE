#pragma once
#include "../Base/DX12LibPCH.h"

constexpr UINT MAXDESCRIPTORS = 1024; //https://learn.microsoft.com/en-us/windows/win32/direct3d12/hardware-support#invariable-limits

struct HandlePair 
{
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

struct DescriptorHeapAllocator
{
    ID3D12DescriptorHeap* Heap = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
    D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
    UINT                        HeapHandleIncrement;
    std::vector<int>            FreeIndices;

    void Create(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> heap)
    {
        if (!(Heap == nullptr && FreeIndices.empty()))
        {
            throw;
        }

        Heap = heap.Get();
        D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
        HeapType = desc.Type;
        HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
                
        if (desc.Flags != D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        {
            throw;
        }

        HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
        HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
        FreeIndices.reserve((int)desc.NumDescriptors);
        for (int n = desc.NumDescriptors; n > 0; n--)
        {
            FreeIndices.push_back(n - 1);
        }            
    }

    void Destroy()
    {
        Heap = nullptr;
        FreeIndices.clear();
    }

    void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
    {
        if (FreeIndices.size() <= 0)
        {
            throw;
        }

        int idx = FreeIndices.back();
        FreeIndices.pop_back();
        out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
        out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
    }

    void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
    {
        int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
        int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);

        if (cpu_idx != gpu_idx)
        {
            throw;
        }

        FreeIndices.push_back(cpu_idx);
    }

    int AllocByIndex()
    {
        if (FreeIndices.size() <= 0)
        {
            throw;
        }

        int idx = FreeIndices.back();
        FreeIndices.pop_back();
        return idx;
    }

    void FreeByIndex(int index)
    {
        FreeIndices.push_back(index);
    }
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

public:
    DescriptorHeapAllocator CbvHeapAllocator;

	void Destroy();

	static void OnInit(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter);
	static void DestroyAll();

    static DescriptorHeaps* GetHeaps(GraphicsAdapter graphicsAdapter);

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index, GraphicsAdapter graphicsAdapter);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index, GraphicsAdapter graphicsAdapter);

	static ComPtr<ID3D12DescriptorHeap> GetCBVHeap(GraphicsAdapter graphicsAdapter);
	static ComPtr<ID3D12DescriptorHeap> GetDSVHeap(GraphicsAdapter graphicsAdapter);

	static int GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE type, GraphicsAdapter graphicsAdapter);
private:
	static int GetNextFreeCBVIndex(GraphicsAdapter graphicsAdapter);
	static int GetNextFreeRTVIndex(GraphicsAdapter graphicsAdapter);
	static int GetNextFreeDSVIndex(GraphicsAdapter graphicsAdapter);
};
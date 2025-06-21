#include "../DescriptorHeaps.h"

static DescriptorHeaps* pSinglePrimaryDevice = nullptr;
static DescriptorHeaps* pSingleSecondDevice = nullptr;

void DescriptorHeaps::LoadCBV(ComPtr<ID3D12Device2> device, DescriptorHeaps* heaps)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		MAXDESCRIPTORS,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heaps->m_cbvHeap));
}

void DescriptorHeaps::LoadSampler(ComPtr<ID3D12Device2> device, DescriptorHeaps* heaps)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		MAXDESCRIPTORS,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heaps->m_samplerHeap));
}

void DescriptorHeaps::LoadRTV(ComPtr<ID3D12Device2> device, DescriptorHeaps* heaps)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		MAXDESCRIPTORS,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heaps->m_rtvHeap)));
}

void DescriptorHeaps::LoadDSV(ComPtr<ID3D12Device2> device, DescriptorHeaps* heaps)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		MAXDESCRIPTORS,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heaps->m_dsvHeap));
}

DescriptorHeaps* DescriptorHeaps::GetHeaps(GraphicsAdapter graphicsAdapter)
{
	if (graphicsAdapter == GraphicAdapterPrimary)
	{
		return pSinglePrimaryDevice;
	}
	else
	{
		return pSingleSecondDevice;
	}
}

void DescriptorHeaps::OnInit(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter)
{
	DescriptorHeaps* heaps = new DescriptorHeaps();

	heaps->m_incrementSize[0] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	heaps->m_incrementSize[1] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	heaps->m_incrementSize[2] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	heaps->m_incrementSize[3] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	LoadCBV(device, heaps);
	LoadDSV(device, heaps);
	LoadRTV(device, heaps);

	if (graphicsAdapter == GraphicAdapterPrimary)
	{
		pSinglePrimaryDevice = heaps;
	}
	else
	{
		pSingleSecondDevice = heaps;
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index, GraphicsAdapter graphicsAdapter)
{
	DescriptorHeaps* heaps = GetHeaps(graphicsAdapter);

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	switch (type) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heaps->m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, heaps->m_incrementSize[0]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heaps->m_samplerHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, heaps->m_incrementSize[1]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heaps->m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, heaps->m_incrementSize[2]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heaps->m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, heaps->m_incrementSize[3]);
		return cpuHandle;
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index, GraphicsAdapter graphicsAdapter)
{
	DescriptorHeaps* heaps = GetHeaps(graphicsAdapter);

	CD3DX12_GPU_DESCRIPTOR_HANDLE cpuHandle;
	switch (type) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		cpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heaps->m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, heaps->m_incrementSize[0]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		cpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heaps->m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, heaps->m_incrementSize[1]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		cpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heaps->m_rtvHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, heaps->m_incrementSize[2]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		cpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(heaps->m_dsvHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, heaps->m_incrementSize[3]);
		return cpuHandle;
	}
}

ComPtr<ID3D12DescriptorHeap> DescriptorHeaps::GetCBVHeap(GraphicsAdapter graphicsAdapter) { return GetHeaps(graphicsAdapter)->m_cbvHeap; }
ComPtr<ID3D12DescriptorHeap> DescriptorHeaps::GetDSVHeap(GraphicsAdapter graphicsAdapter) { return GetHeaps(graphicsAdapter)->m_dsvHeap; }

HandlePair DescriptorHeaps::BatchHandles(D3D12_DESCRIPTOR_HEAP_TYPE type, GraphicsAdapter graphicsAdapter)
{
	DescriptorHeaps* heaps = GetHeaps(graphicsAdapter);
	return { GetCPUHandle(type, heaps->m_usedSize[type], graphicsAdapter), GetGPUHandle(type, heaps->m_usedSize[type]++, graphicsAdapter) };
}

int DescriptorHeaps::GetNextFreeIndex(D3D12_DESCRIPTOR_HEAP_TYPE type, GraphicsAdapter graphicsAdapter)
{
	switch (type)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return GetNextFreeCBVIndex(graphicsAdapter);
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return GetNextFreeRTVIndex(graphicsAdapter);
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return GetNextFreeDSVIndex(graphicsAdapter);
	default:
		break;
	}

	return -1;
}

int DescriptorHeaps::GetNextFreeCBVIndex(GraphicsAdapter graphicsAdapter)
{
	static int CBVindexPrimary = 0;
	static int CBVindexSecond = 0;

	if (graphicsAdapter == GraphicAdapterPrimary)
	{
		return CBVindexPrimary++;
	}
	else
	{
		return CBVindexSecond++;
	}
}

int DescriptorHeaps::GetNextFreeRTVIndex(GraphicsAdapter graphicsAdapter)
{
	static int RTVindexPrimary = 0;
	static int RTVindexSecond = 0;

	if (graphicsAdapter == GraphicAdapterPrimary)
	{
		return RTVindexPrimary++;
	}
	else
	{
		return RTVindexSecond++;
	}
}

int DescriptorHeaps::GetNextFreeDSVIndex(GraphicsAdapter graphicsAdapter)
{
	static int DSVindexPrimary = 0;
	static int DSVindexSecond = 0;

	if (graphicsAdapter == GraphicAdapterPrimary)
	{
		return DSVindexPrimary++;
	}
	else
	{
		return DSVindexSecond++;
	}
}

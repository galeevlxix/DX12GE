#include "DescriptorHeaps.h"

static DescriptorHeaps* pSingle = nullptr;

void DescriptorHeaps::LoadCBV(ComPtr<ID3D12Device2> device)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		MAXDESCRIPTORS,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSingle->m_cbvHeap));
}

void DescriptorHeaps::LoadSampler(ComPtr<ID3D12Device2> device)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		MAXDESCRIPTORS,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	};
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSingle->m_samplerHeap));
}

void DescriptorHeaps::LoadRTV(ComPtr<ID3D12Device2> device) 
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		MAXDESCRIPTORS,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSingle->m_rtvHeap)));
}

void DescriptorHeaps::LoadDSV(ComPtr<ID3D12Device2> device) 
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		MAXDESCRIPTORS,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&pSingle->m_dsvHeap));
}

void DescriptorHeaps::OnInit(ComPtr<ID3D12Device2> device) 
{
	if (!pSingle)
	{
		pSingle = new DescriptorHeaps();
	}

	pSingle->m_incrementSize[0] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	pSingle->m_incrementSize[1] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	pSingle->m_incrementSize[2] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	pSingle->m_incrementSize[3] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	LoadCBV(device);
	LoadDSV(device);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index) 
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	switch (type) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(pSingle->m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, pSingle->m_incrementSize[0]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(pSingle->m_samplerHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, pSingle->m_incrementSize[1]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(pSingle->m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, pSingle->m_incrementSize[2]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(pSingle->m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, pSingle->m_incrementSize[3]);
		return cpuHandle;
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT index) 
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE cpuHandle;
	switch (type) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		cpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(pSingle->m_cbvHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, pSingle->m_incrementSize[0]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		cpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(pSingle->m_samplerHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, pSingle->m_incrementSize[1]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		cpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(pSingle->m_rtvHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, pSingle->m_incrementSize[2]);
		return cpuHandle;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		cpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(pSingle->m_dsvHeap->GetGPUDescriptorHandleForHeapStart());
		cpuHandle.Offset(index, pSingle->m_incrementSize[3]);
		return cpuHandle;
	}
}

ComPtr<ID3D12DescriptorHeap> DescriptorHeaps::GetCBVHeap() { return pSingle->m_cbvHeap; }
ComPtr<ID3D12DescriptorHeap> DescriptorHeaps::GetDSVHeap() { return pSingle->m_dsvHeap; }

HandlePair DescriptorHeaps::BatchHandles(D3D12_DESCRIPTOR_HEAP_TYPE type) 
{
	//printf("%d %d\n", type, m_usedSize[type]);
	return { GetCPUHandle(type, pSingle->m_usedSize[type]), GetGPUHandle(type, pSingle->m_usedSize[type]++) };
}
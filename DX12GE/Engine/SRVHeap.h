#pragma once
#include "DX12LibPCH.h"

class SRVHeap
{
public:
	static void Create();
	static SRVHeap* GetHeap();
	
	void AddResource(ComPtr<ID3D12Resource> resource, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc);
	void Set(ComPtr<ID3D12GraphicsCommandList2> commandList);

protected:
	SRVHeap();

private:
	ComPtr<ID3D12DescriptorHeap> m_SRVHeap;
	UINT m_DescriptorSize;
	ComPtr<ID3D12Device2> m_Device;
};
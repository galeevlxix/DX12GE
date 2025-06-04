#pragma once
#include "../Base/DX12LibPCH.h"
#include "ShaderResources.h"

struct Target
{
	ComPtr<ID3D12Resource> Texture;

	D3D12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle;

	int RtvHeapIndex = -1;
	int SrvHeapIndex = -1;
};

class GBuffer
{
public:
	enum TargetType
	{
		POSITION = 0,
		NORMAL = 1,
		DIFFUSE = 2,
		EMISSIVE = 3,
		ORM = 4
	};

	void Init(ComPtr<ID3D12Device2> device, UINT width, UINT height);
	void Release();
	void Resize(UINT width, UINT height);
	void BindRenderTargets(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE SrvGPU(TargetType type) const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvCPU(TargetType type) const;

	void SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetGraphicsRootDescriptorTables(int fromSlot, ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	UINT m_Width = 0;
	UINT m_Height = 0;
	ComPtr<ID3D12Device2> m_Device;

	Target m_Targets[GBUFFER_COUNT];

	void BuildResources();
	void BuildDescriptors();
};
#pragma once
#include "DX12LibPCH.h"

struct Target
{
	ComPtr<ID3D12Resource> Texture;

	D3D12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle;
};

class GBuffer
{
public:
	static const UINT GBUFFER_COUNT = 3;

	enum TargetType
	{
		POSITION = 0,
		NORMAL = 1,
		DIFFUSE = 2,
	};

	void Init(ComPtr<ID3D12Device2> device, UINT width, UINT height, int cpuDescHandleOffset);
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
	UINT HandleOffset;

	Target m_Targets[GBUFFER_COUNT];

	void BuildResources();
	void BuildDescriptors();
};
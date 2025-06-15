#pragma once

#include "TextureComponent.h"

static const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

class TextureBuffer
{
public:
	void Init(ComPtr<ID3D12Device2> device, UINT width, UINT height, DXGI_FORMAT format);
	void Release();
	void Resize(UINT width, UINT height);
	void BindRenderTarget(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE SrvGPU() const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvCPU() const;

	void SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetGraphicsRootDescriptorTable(int slot, ComPtr<ID3D12GraphicsCommandList2> commandList);

private:

	UINT m_Width = 0;
	UINT m_Height = 0;
	ComPtr<ID3D12Device2> m_Device;
	DXGI_FORMAT m_Format;

	ComPtr<ID3D12Resource> Buffer;

	D3D12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle;

	int RtvHeapIndex = -1;
	int SrvHeapIndex = -1;

	void BuildResource();
	void BuildDescriptors();
};
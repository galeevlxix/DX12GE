#pragma once

#include "TextureComponent.h"

static const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

class TextureBuffer
{
private:
	UINT m_Width = 0;
	UINT m_Height = 0;

	ComPtr<ID3D12Device2> m_Device;
	GraphicsAdapter m_Adapter;
	
	DXGI_FORMAT m_Format;
	bool m_CreateReadback;
	
	ComPtr<ID3D12Resource> m_Resource;
	ComPtr<ID3D12Resource> m_ReadbackBuffer;

	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuRtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandle;

	LPCWSTR m_Name = L"";

	D3D12_RESOURCE_STATES m_CurrentState;

	int m_RtvHeapIndex = -1;
	int m_SrvHeapIndex = -1;

public:
	TextureBuffer();
	void Init(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter, UINT width, UINT height, DXGI_FORMAT format, bool createReadback);
	void Init(ComPtr<ID3D12Device2> device, D3D12_RESOURCE_DESC& resourceDesc, ComPtr<ID3D12Heap> heap, const std::wstring& name, bool createReadback);
	void Destroy();
	void Resize(UINT width, UINT height);
	void BindRenderTarget(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE SrvGPU() const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_GpuSrvHandle); }
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvCPU() const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CpuRtvHandle); }

	void SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToCopyDest(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToCopySource(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToState(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_RESOURCE_STATES newState);

	void SetGraphicsRootDescriptorTable(int slot, ComPtr<ID3D12GraphicsCommandList2> commandList);

	ComPtr<ID3D12Resource> GetResource() { return m_Resource; }
	ComPtr<ID3D12Resource> GetReadbackBuffer() { return m_ReadbackBuffer; }
	ComPtr<ID3D12Device2> GetDevice() { return m_Device; }
	void SetName(LPCWSTR name) { m_Name = name; }
	wstring GetName() { return m_Name; }
	D3D12_RESOURCE_STATES GetResourceState() { return m_CurrentState; }
	void SetAdapter(GraphicsAdapter graphicsAdapter) { m_Adapter = graphicsAdapter; }
	UINT GetWidth() { return m_Width; }
	UINT GetHeight() { return m_Height; }

private:

	void BuildResource();
	void BuildDescriptors();
};
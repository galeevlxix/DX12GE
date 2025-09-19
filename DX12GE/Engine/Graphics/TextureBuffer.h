#pragma once

#include "TextureComponent.h"

static const FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

class TextureBuffer
{
public:
	TextureBuffer();
	void Init(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter, UINT width, UINT height, DXGI_FORMAT format);
	void Init(ComPtr<ID3D12Device2> device, D3D12_RESOURCE_DESC& resourceDesc, ComPtr<ID3D12Heap> heap, const std::wstring& name);
	void Release();
	void Resize(UINT width, UINT height);
	void BindRenderTarget(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE SrvGPU() const { return CD3DX12_GPU_DESCRIPTOR_HANDLE(GpuSrvHandle); }
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvCPU() const { return CD3DX12_CPU_DESCRIPTOR_HANDLE(CpuRtvHandle); }

	void SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToCopyDest(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToCopySource(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToState(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_RESOURCE_STATES newState);

	void SetGraphicsRootDescriptorTable(int slot, ComPtr<ID3D12GraphicsCommandList2> commandList);

	ComPtr<ID3D12Resource> GetResource() { return Buffer; }
	ComPtr<ID3D12Device2> GetDevice() { return m_Device; }
	void SetName(LPCWSTR name) { m_Name = name; }
	wstring GetName() { return m_Name; }
	D3D12_RESOURCE_STATES GetResourceState() { return currentState; }
	void SetAdapter(GraphicsAdapter graphicsAdapter) { m_Adapter = graphicsAdapter; }

private:
	UINT m_Width = 0;
	UINT m_Height = 0;

	ComPtr<ID3D12Device2> m_Device;
	GraphicsAdapter m_Adapter;

	DXGI_FORMAT m_Format;

	ComPtr<ID3D12Resource> Buffer;

	D3D12_CPU_DESCRIPTOR_HANDLE CpuRtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuSrvHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE GpuSrvHandle;

	LPCWSTR m_Name = L"";

	D3D12_RESOURCE_STATES currentState;

	int RtvHeapIndex = -1;
	int SrvHeapIndex = -1;

	void BuildResource();
	void BuildDescriptors();
};
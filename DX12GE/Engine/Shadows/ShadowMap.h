#pragma once

#include "../DX12LibPCH.h"
#include "../Texture.h"

class ShadowMap
{
public:
	ShadowMap(ComPtr<ID3D12Device2> device, UINT width, UINT height);
	ShadowMap(const ShadowMap& rhs) = delete;
	ShadowMap& operator=(const ShadowMap& rhs) = delete;
	~ShadowMap() = default;

	UINT Width() const;
	UINT Height() const;

	ID3D12Resource* Resource();

	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv() const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv() const;

	D3D12_VIEWPORT Viewport() const;
	D3D12_RECT ScissorRect() const;

	void OnResize(UINT newWidth, UINT newHeight);

	void SetToWrite(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	void BuildDescriptors();
	void BuildResource();

	ComPtr<ID3D12Device2> md3dDevice = nullptr;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT m_Width = 0;
	UINT m_Height = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuSrvHadle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GpuSrvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuDsvHandle;

	int m_SrvHeapIndex;
	int m_DsvHeapIndex;

	Texture m_Texture;
};
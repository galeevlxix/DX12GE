#pragma once

#include "../DX12LibPCH.h"

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

	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
	void BuildDescriptors();
	void BuildResource();

	ComPtr<ID3D12Device2> md3dDevice = nullptr;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;

	ComPtr<ID3D12DescriptorHeap> m_DSVHeap;
	ComPtr<ID3D12DescriptorHeap> m_SRVHeap;

	ComPtr<ID3D12Resource> mShadowMap = nullptr;
};
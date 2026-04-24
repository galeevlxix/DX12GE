#pragma once
#include "TextureBuffer.h"
#include "../Base/Application.h"

class CrossAdapterResource
{
	std::shared_ptr<TextureBuffer> m_PrimarySharedResource;
	std::shared_ptr<TextureBuffer> m_SecondSharedResource;
	ComPtr<ID3D12Heap> m_CrossAdapterResourceHeap[2];

public:

	CrossAdapterResource(
		ComPtr<ID3D12Device2> primaryDevice, 
		ComPtr<ID3D12Device2> secondDevice,
		D3D12_RESOURCE_DESC resourceDesc,
		const std::wstring& name);

	std::shared_ptr<TextureBuffer> GetPrimarySharedResource();
	std::shared_ptr<TextureBuffer> GetSecondSharedResource();

	void Resize(const UINT newWidth, const UINT newHeight);

	void Destroy();
};
#pragma once
#include "TextureBuffer.h"
#include "../Base/Application.h"

class CrossAdapterResource
{
private:
	std::shared_ptr<TextureBuffer> m_PrimeResource;
	std::shared_ptr<TextureBuffer> m_SharedResource;

	ComPtr<ID3D12Heap> crossAdapterResourceHeap[2];

	bool isInit = false;

public:

	CrossAdapterResource(
		ComPtr<ID3D12Device2> primaryDevice, 
		ComPtr<ID3D12Device2> secondDevice,
		D3D12_RESOURCE_DESC& resourceDesc,
		const std::wstring& name);

	std::shared_ptr<TextureBuffer> GetPrimeResource();
	std::shared_ptr<TextureBuffer> GetSharedResource();

	void Resize(const UINT newWidth, const UINT newHeight);
};
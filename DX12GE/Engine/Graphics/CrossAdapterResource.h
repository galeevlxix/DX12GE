#pragma once
#include "TextureBuffer.h"
#include "../Base/Application.h"

class CrossAdapterResource
{
private:
	TextureBuffer m_PrimeResource;
	TextureBuffer m_SharedResource;

	CrossAdapterResource(
		ComPtr<ID3D12Device2> primaryDevice, 
		ComPtr<ID3D12Device2> secondDevice,
		UINT width, UINT height, DXGI_FORMAT format);


};
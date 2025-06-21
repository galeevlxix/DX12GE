#pragma once
#include "../Base/DX12LibPCH.h"
#include "ShaderResources.h"
#include "TextureBuffer.h"

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

	void Init(ComPtr<ID3D12Device2> device, GraphicsAdapter graphicsAdapter, UINT width, UINT height);
	void Release();
	void Resize(UINT width, UINT height);
	void BindRenderTargets(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE SrvGPU(TargetType type) const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvCPU(TargetType type) const;

	void SetToWriteAndClear(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetToRead(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void SetGraphicsRootDescriptorTable(int slot, TargetType type, ComPtr<ID3D12GraphicsCommandList2> commandList);

	std::shared_ptr<TextureBuffer> GetBuffer(TargetType type);

private:
	std::shared_ptr<TextureBuffer> m_Targets[GBUFFER_COUNT];
};
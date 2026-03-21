#pragma once

#include "../Base/DX12LibPCH.h"
#include <string>

class TextureComponent
{
public:
    ComPtr<ID3D12Resource> m_Resource;
    int m_SRVHeapIndex = -1;

private:
    ComPtr<ID3D12Resource> m_UploadBuffer;
    bool m_Initialized = false;

    std::string m_ResourcePath;

public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& path);
    void OnLoadCubemap(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& path);

    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot);
    void Destroy();

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

	const std::string& GetResourcePath() { return m_ResourcePath; }

    bool IsInitialized();
};
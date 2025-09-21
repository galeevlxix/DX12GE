#pragma once

#include "../Base/DX12LibPCH.h"
#include <string>

using namespace std;

enum TextureType
{
    NONE = 0,
    DIFFUSE = 1,
    SPECULAR = 2,
    AMBIENT = 3,
    EMISSIVE = 4,
    HEIGHT = 5,
    NORMALS = 6,
    SHININESS = 7,
    OPACITY = 8,
    DISPLACEMENT = 9,
    LIGHTMAP = 10,
    REFLECTION = 11,
    BASE_COLOR = 12,
    NORMAL_CAMERA = 13,
    EMISSION_COLOR = 14,
    METALNESS = 15,
    DIFFUSE_ROUGHNESS = 16,
    AMBIENT_OCCLUSION = 17,
    UNKNOWN = 18,
    SHEEN = 19,
    CLEARCOAT = 20,
    TRANSMISSION = 21,
    MAYA_BASE = 22,
    MAYA_SPECULAR = 23,
    MAYA_SPECULAR_COLOR = 24,
    MAYA_SPECULAR_ROUGHNESS = 25,
    ANISOTROPY = 26,
    GLTF_METALLIC_ROUGHNESS = 27,
};

class TextureComponent
{
public:
    ComPtr<ID3D12Resource> m_Resource;
    int m_SRVHeapIndex;

private:
    ComPtr<ID3D12Resource> m_UploadBuffer;
    bool m_Initialized = false;

public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, string path);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot);
    void Destroy();

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

    bool IsInitialized();
};
#pragma once

#include <map>
#include "../Base/DX12LibPCH.h"
#include "../Base/SimpleMath.h"
#include "TextureComponent.h"

class MaterialEntity
{
private:
    uint64_t m_DiffuseTextureId = -1;
    uint64_t m_EmissiveTextureId = -1;
    uint64_t m_NormalTextureId = -1;
    uint64_t m_MetallicTextureId = -1;
    uint64_t m_RoughnessTextureId = -1;
    uint64_t m_GltfMetallicRoughnessTextureId = -1;
    uint64_t m_AOTextureId = -1;

    bool DrawIt = true;

    DirectX::SimpleMath::Vector4 HasDiffuseNormalEmissive = DirectX::SimpleMath::Vector4(0, 0, 0, 0);
    DirectX::SimpleMath::Vector4 HasOcclusionRoughnessMetallicCombined = DirectX::SimpleMath::Vector4(0, 0, 0, 0);

    uint64_t AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, std::string &path);

public:
    map<TextureType, string> m_ImagePaths;

    void Load(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot, uint64_t textureId, float mask);
    void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);

    bool CanDrawIt() { return DrawIt; }
};
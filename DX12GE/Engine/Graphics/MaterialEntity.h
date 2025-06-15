#pragma once

#include <map>
#include "../Base/DX12LibPCH.h"
#include "../Base/SimpleMath.h"
#include "TextureComponent.h"

class MaterialEntity
{
private:
    uint32_t m_DiffuseTextureId;
    uint32_t m_EmissiveTextureId;
    uint32_t m_NormalTextureId;
    uint32_t m_MetallicTextureId;
    uint32_t m_RoughnessTextureId;
    uint32_t m_GltfMetallicRoughnessTextureId;
    uint32_t m_AOTextureId;

    bool DrawIt = true;

    DirectX::SimpleMath::Vector4 HasDiffuseNormalEmissive = DirectX::SimpleMath::Vector4(0, 0, 0, 0);
    DirectX::SimpleMath::Vector4 HasOcclusionRoughnessMetallicCombined = DirectX::SimpleMath::Vector4(0, 0, 0, 0);

    uint32_t AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, std::string &path);

public:
    map<TextureType, string> m_ImagePaths;

    void Load(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot, uint32_t textureId, float mask);
    void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);

    bool CanDrawIt() { return DrawIt; }
};
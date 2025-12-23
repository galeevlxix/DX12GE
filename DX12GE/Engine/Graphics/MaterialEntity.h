#pragma once

#include <map>
#include "../Base/DX12LibPCH.h"
#include "../Base/SimpleMath.h"
#include "TextureComponent.h"

class MaterialEntity
{
private:
    uint32_t m_DiffuseTextureId = -1;
    uint32_t m_EmissiveTextureId = -1;
    uint32_t m_NormalTextureId = -1;
    uint32_t m_MetallicTextureId = -1;
    uint32_t m_RoughnessTextureId = -1;
    uint32_t m_GltfMetallicRoughnessTextureId = -1;
    uint32_t m_AOTextureId = -1;

    DirectX::SimpleMath::Vector4 m_HasDiffuseNormalEmissive = DirectX::SimpleMath::Vector4(0, 0, 0, 0);
    DirectX::SimpleMath::Vector4 m_HasOcclusionRoughnessMetallicCombined = DirectX::SimpleMath::Vector4(0, 0, 0, 0);

    bool m_DrawIt = false;

    uint32_t AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, std::string &path);
    void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot, uint32_t textureId, float mask);
    void DestroyTexture(uint32_t textureId);

public:
    std::map<TextureType, std::string> m_ImagePaths;

    void Load(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);
    bool CanDrawIt() { return m_DrawIt; }

    void Destroy();
};
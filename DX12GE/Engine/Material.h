#pragma once

#include <map>
#include "DX12LibPCH.h"
#include "Texture.h"
#include "SimpleMath.h"

class Material
{
private:
    Texture* m_DiffuseTexture;
    Texture* m_EmissiveTexture;
    Texture* m_NormalTexture;
    Texture* m_MetallicTexture;
    Texture* m_RoughnessTexture;
    Texture* m_GltfMetallicRoughnessTexture;
    Texture* m_AOTexture;

    bool DrawIt = true;

    DirectX::SimpleMath::Vector4 HasDiffuseNormalEmissive = DirectX::SimpleMath::Vector4(0, 0, 0, 0);
    DirectX::SimpleMath::Vector4 HasOcclusionRoughnessMetallicCombined = DirectX::SimpleMath::Vector4(0, 0, 0, 0);

    Texture* AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, const char* path);    

public:
    map<TextureType, string> m_ImagePaths;

    void Load(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot, Texture* texture, float mask);
    void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);

    bool CanDrawIt() { return DrawIt; }
};
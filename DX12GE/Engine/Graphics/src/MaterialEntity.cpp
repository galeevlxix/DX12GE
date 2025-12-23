#include "../MaterialEntity.h"
#include "../ShaderResources.h"
#include "../ResourceStorage.h"

uint32_t MaterialEntity::AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, std::string& path)
{
    if (NotFoundFile(path.c_str()))
    {
        path = "empty";
        return -1;
    }

    int id = ResourceStorage::AddTexture(path);
    std::shared_ptr<TextureComponent> texture = ResourceStorage::GetTexture(id);

    if (!texture->IsInitialized())
        texture->OnLoad(commandList, path);
    
    return id;
}

int IsNotNull(uint32_t id)
{
    return id == -1 ? 0 : 1;
}

void MaterialEntity::Load(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    m_DiffuseTextureId =                AddTexture(commandList, m_ImagePaths[TextureType::DIFFUSE]);
    m_NormalTextureId =                 AddTexture(commandList, m_ImagePaths[TextureType::NORMALS]);
    m_EmissiveTextureId =               AddTexture(commandList, m_ImagePaths[TextureType::EMISSIVE]);
    m_MetallicTextureId =               AddTexture(commandList, m_ImagePaths[TextureType::METALNESS]);
    m_RoughnessTextureId =              AddTexture(commandList, m_ImagePaths[TextureType::DIFFUSE_ROUGHNESS]);
    m_GltfMetallicRoughnessTextureId =  AddTexture(commandList, m_ImagePaths[TextureType::GLTF_METALLIC_ROUGHNESS]);
    m_AOTextureId =                     AddTexture(commandList, m_ImagePaths[TextureType::AMBIENT_OCCLUSION]);

    m_HasDiffuseNormalEmissive.x = static_cast<float>(IsNotNull(m_DiffuseTextureId));
    m_HasDiffuseNormalEmissive.y = static_cast<float>(IsNotNull(m_NormalTextureId));
    m_HasDiffuseNormalEmissive.z = static_cast<float>(IsNotNull(m_EmissiveTextureId));
    m_HasDiffuseNormalEmissive.w = 1.234567f;

    m_HasOcclusionRoughnessMetallicCombined.x = static_cast<float>(IsNotNull(m_AOTextureId));
    m_HasOcclusionRoughnessMetallicCombined.w = static_cast<float>(IsNotNull(m_GltfMetallicRoughnessTextureId));
    if (m_HasOcclusionRoughnessMetallicCombined.w < 0.5f)
    {
        m_HasOcclusionRoughnessMetallicCombined.y = static_cast<float>(IsNotNull(m_RoughnessTextureId));
        m_HasOcclusionRoughnessMetallicCombined.z = static_cast<float>(IsNotNull(m_MetallicTextureId));
    }

    m_DrawIt = true;
}

void MaterialEntity::RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot, uint32_t textureId, float mask)
{
    if (mask > 0.5f) ResourceStorage::GetTexture(textureId)->OnRender(commandList, slot);
}

void MaterialEntity::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    ShaderResources::GetGeometryPassCB()->HasDiffuseNormalEmissive = m_HasDiffuseNormalEmissive;
    ShaderResources::GetGeometryPassCB()->HasOcclusionRoughnessMetallicCombined = m_HasOcclusionRoughnessMetallicCombined;
    ShaderResources::SetGeometryPassCB(commandList, 1);

    RenderTexture(commandList, 2, m_DiffuseTextureId,     m_HasDiffuseNormalEmissive.x);
    RenderTexture(commandList, 3, m_NormalTextureId,      m_HasDiffuseNormalEmissive.y);
    RenderTexture(commandList, 4, m_EmissiveTextureId,    m_HasDiffuseNormalEmissive.z);

    RenderTexture(commandList, 5, m_AOTextureId,          m_HasOcclusionRoughnessMetallicCombined.x);
    RenderTexture(commandList, 6, m_RoughnessTextureId,   m_HasOcclusionRoughnessMetallicCombined.y);
    RenderTexture(commandList, 7, m_MetallicTextureId,    m_HasOcclusionRoughnessMetallicCombined.z);
    RenderTexture(commandList, 8, m_GltfMetallicRoughnessTextureId,m_HasOcclusionRoughnessMetallicCombined.w);
}

void MaterialEntity::DestroyTexture(uint32_t textureId)
{
    if (textureId == -1) return;
    ResourceStorage::DeleteTextureComponentForever(textureId);
}

void MaterialEntity::Destroy()
{
    if (!m_DrawIt) return;
    DestroyTexture(m_DiffuseTextureId);
    DestroyTexture(m_EmissiveTextureId);
    DestroyTexture(m_NormalTextureId);
    DestroyTexture(m_MetallicTextureId);
    DestroyTexture(m_RoughnessTextureId);
    DestroyTexture(m_GltfMetallicRoughnessTextureId);
    DestroyTexture(m_AOTextureId);
    m_ImagePaths.clear();
    m_DrawIt = false;
}

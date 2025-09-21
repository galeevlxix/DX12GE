#include "../MaterialEntity.h"
#include "../ShaderResources.h"
#include "../ResourceStorage.h"

uint32_t MaterialEntity::AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, std::string& path)
{
    if (NotFoundFile(path.c_str())) return -1;

    int id = ResourceStorage::AddTexture(path);
    auto texture = ResourceStorage::GetTexture(id);

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

    HasDiffuseNormalEmissive.x = static_cast<float>(IsNotNull(m_DiffuseTextureId));
    HasDiffuseNormalEmissive.y = static_cast<float>(IsNotNull(m_NormalTextureId));
    HasDiffuseNormalEmissive.z = static_cast<float>(IsNotNull(m_EmissiveTextureId));
    HasDiffuseNormalEmissive.w = 1.234567f;

    HasOcclusionRoughnessMetallicCombined.x = static_cast<float>(IsNotNull(m_AOTextureId));
    HasOcclusionRoughnessMetallicCombined.w = static_cast<float>(IsNotNull(m_GltfMetallicRoughnessTextureId));
    if (HasOcclusionRoughnessMetallicCombined.w < 0.5f)
    {
        HasOcclusionRoughnessMetallicCombined.y = static_cast<float>(IsNotNull(m_RoughnessTextureId));
        HasOcclusionRoughnessMetallicCombined.z = static_cast<float>(IsNotNull(m_MetallicTextureId));
    }
}

void MaterialEntity::RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot, uint32_t textureId, float mask)
{
    if (mask > 0.5f) ResourceStorage::GetTexture(textureId)->OnRender(commandList, slot);
}

void MaterialEntity::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    ShaderResources::GetMaterialCB()->HasDiffuseNormalEmissive = HasDiffuseNormalEmissive;
    ShaderResources::GetMaterialCB()->HasOcclusionRoughnessMetallicCombined = HasOcclusionRoughnessMetallicCombined;
    ShaderResources::SetMaterialCB(commandList, 1);

    RenderTexture(commandList, 2, m_DiffuseTextureId,     HasDiffuseNormalEmissive.x);
    RenderTexture(commandList, 3, m_NormalTextureId,      HasDiffuseNormalEmissive.y);
    RenderTexture(commandList, 4, m_EmissiveTextureId,    HasDiffuseNormalEmissive.z);

    RenderTexture(commandList, 5, m_AOTextureId,          HasOcclusionRoughnessMetallicCombined.x);
    RenderTexture(commandList, 6, m_RoughnessTextureId,   HasOcclusionRoughnessMetallicCombined.y);
    RenderTexture(commandList, 7, m_MetallicTextureId,    HasOcclusionRoughnessMetallicCombined.z);
    RenderTexture(commandList, 8, m_GltfMetallicRoughnessTextureId,HasOcclusionRoughnessMetallicCombined.w);
}
#include "Material.h"
#include <io.h>
#include "ShaderResources.h"

bool FileExists(const char* fname)
{
    return _access(fname, 0) != -1;
}

bool NotFoundTexture(const char* path)
{
    return path == "" || !FileExists(path);
}

Texture* Material::AddTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, const char* path)
{
    if (NotFoundTexture(path)) return nullptr;
    Texture* texture = new Texture();
    texture->Load(commandList, path);
    return texture;
}

int IsNotNull(Texture* texture)
{
    return texture == nullptr ? 0 : 1;
}

void Material::Load(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    m_DiffuseTexture = AddTexture(commandList, m_ImagePaths[TextureType::DIFFUSE].c_str());
    m_NormalTexture = AddTexture(commandList, m_ImagePaths[TextureType::NORMALS].c_str());
    m_EmissiveTexture = AddTexture(commandList, m_ImagePaths[TextureType::EMISSIVE].c_str());
    m_MetallicTexture = AddTexture(commandList, m_ImagePaths[TextureType::METALNESS].c_str());
    m_RoughnessTexture = AddTexture(commandList, m_ImagePaths[TextureType::DIFFUSE_ROUGHNESS].c_str());
    m_GltfMetallicRoughnessTexture = AddTexture(commandList, m_ImagePaths[TextureType::GLTF_METALLIC_ROUGHNESS].c_str());
    m_AOTexture = AddTexture(commandList, m_ImagePaths[TextureType::AMBIENT_OCCLUSION].c_str());

    HasDiffuseNormalEmissive.x = IsNotNull(m_DiffuseTexture);
    HasDiffuseNormalEmissive.y = IsNotNull(m_NormalTexture);
    HasDiffuseNormalEmissive.z = IsNotNull(m_EmissiveTexture);
    HasDiffuseNormalEmissive.w = 1.234567f;

    HasOcclusionRoughnessMetallicCombined.x = IsNotNull(m_AOTexture);
    HasOcclusionRoughnessMetallicCombined.w = IsNotNull(m_GltfMetallicRoughnessTexture);
    if (HasOcclusionRoughnessMetallicCombined.w < 0.5f)
    {
        HasOcclusionRoughnessMetallicCombined.y = IsNotNull(m_RoughnessTexture);
        HasOcclusionRoughnessMetallicCombined.z = IsNotNull(m_MetallicTexture);
    }
}

void Material::RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot, Texture* texture, float mask)
{
    if (mask > 0.5f) texture->Render(commandList, slot);
}

void Material::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    ShaderResources::GetMaterialCB()->HasDiffuseNormalEmissive = HasDiffuseNormalEmissive;
    ShaderResources::GetMaterialCB()->HasOcclusionRoughnessMetallicCombined = HasOcclusionRoughnessMetallicCombined;
    ShaderResources::SetMaterialCB(commandList, 1);

    RenderTexture(commandList, 2, m_DiffuseTexture,     HasDiffuseNormalEmissive.x);
    RenderTexture(commandList, 3, m_NormalTexture,      HasDiffuseNormalEmissive.y);
    RenderTexture(commandList, 4, m_EmissiveTexture,    HasDiffuseNormalEmissive.z);

    RenderTexture(commandList, 5, m_AOTexture,          HasOcclusionRoughnessMetallicCombined.x);
    RenderTexture(commandList, 6, m_RoughnessTexture,   HasOcclusionRoughnessMetallicCombined.y);
    RenderTexture(commandList, 7, m_MetallicTexture,    HasOcclusionRoughnessMetallicCombined.z);
    RenderTexture(commandList, 8, m_GltfMetallicRoughnessTexture,HasOcclusionRoughnessMetallicCombined.w);
}
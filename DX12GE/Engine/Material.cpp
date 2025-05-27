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
    if (m_ImagePaths.size() == 0 || NotFoundTexture(m_ImagePaths[TextureType::DIFFUSE].c_str()))
    {
        m_DiffuseTexture = new Texture();
        m_DiffuseTexture->Load(commandList, "../../DX12GE/Resources/empty.png");
        return;
    }

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
    HasMetallicRoughnessOcclusion.x = IsNotNull(m_MetallicTexture);
    HasMetallicRoughnessOcclusion.y = IsNotNull(m_RoughnessTexture);
    HasMetallicRoughnessOcclusion.z = IsNotNull(m_GltfMetallicRoughnessTexture);
    HasMetallicRoughnessOcclusion.w = IsNotNull(m_AOTexture);
}

void Material::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    ShaderResources::GetMaterialCB()->HasDiffuseNormalEmissive = HasDiffuseNormalEmissive;
    ShaderResources::GetMaterialCB()->HasMetallicRoughnessOcclusion = HasMetallicRoughnessOcclusion;
    ShaderResources::SetMaterialCB(commandList, 1);

    m_DiffuseTexture->Render(commandList, 2);
    if (HasDiffuseNormalEmissive.y > 0.5f) m_NormalTexture->Render(commandList, 3);
}
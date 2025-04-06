#include "Material.h"

void Material::Load(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    // load image data from disk
    string path = " ";
    if (m_ImagePaths.size() != 0)
    {
        path = m_ImagePaths[TextureType::DIFFUSE];
    }
    else
    {
        DrawIt = false;
        return;
    }

    m_DiffuseTexture.Load(commandList, path);
}

void Material::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    m_DiffuseTexture.Render(commandList);
}
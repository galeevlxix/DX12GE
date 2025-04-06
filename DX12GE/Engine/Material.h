#pragma once

#include <map>
#include "DX12LibPCH.h"
#include "Texture.h"

class Material
{
private:
    Texture m_DiffuseTexture;    
    bool DrawIt = true;

public:
    map<TextureType, string> m_ImagePaths;

    void Load(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);

    bool CanDrawIt() { return DrawIt; }
};
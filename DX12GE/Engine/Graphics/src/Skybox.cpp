#include "../Skybox.h"
#include "../ShaderResources.h"
#include <vector>

void Skybox::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    std::vector<XMFLOAT3> cubeVertices =
    {
        {-1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f,  1.0f},
        {-1.0f,  1.0f,  1.0f},
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f,  1.0f},
        {-1.0f, -1.0f,  1.0f}
    };

    std::vector<WORD> indices =
    {
        // Front face
        0, 1, 2,
        0, 2, 3,

        // Back face
        5, 4, 7,
        5, 7, 6,

        // Left face
        4, 0, 3,
        4, 3, 7,

        // Right face
        1, 5, 6,
        1, 6, 2,

        // Top face
        4, 5, 1,
        4, 1, 0,

        // Bottom face
        3, 2, 6,
        3, 6, 7
    };

	m_Texture.OnLoadCubemap(commandList, "../../DX12GE/Resources/Skybox Textures/snowcube1024.dds");
    m_BoxMesh.OnLoad(commandList, cubeVertices, indices);
    m_Transform.SetDefault(0.0f);
    m_Transform.SetScale(500.0f);
}

void Skybox::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix)
{
    XMMATRIX wvp = m_Transform.GetWorldMatrix();
    XMMATRIX mvp = XMMatrixMultiply(wvp, viewProjMatrix);    
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvp, 0);

    RenderTexture(commandList, 1);
	m_BoxMesh.OnRender(commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Skybox::RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot)
{
    m_Texture.OnRender(commandList, slot);
}

void Skybox::Destroy()
{
    m_BoxMesh.Destroy();
    m_Texture.Destroy();
}
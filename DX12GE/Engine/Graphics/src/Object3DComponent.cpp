#include "../Object3DComponent.h"
#include "../CurrentPass.h"
#include "../../Base/DX12LibPCH.h"

void Object3DComponent::OnLoad(std::vector<Mesh3DComponent*>& meshes)
{
    m_Meshes = meshes;

    bool init = true;
    for (auto mesh : m_Meshes)
    {
        init &= mesh->IsInitialized();
    }
    m_Initialized = init;
}

void Object3DComponent::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (!m_Initialized) return;

    for (int i = 0; i < m_Meshes.size(); i++)
    {
        if (!m_Meshes[i]->Material->CanDrawIt()) continue;

        if (CurrentPass::Get() != CurrentPass::Shadow && CurrentPass::Get() != CurrentPass::Lighting)
            m_Meshes[i]->Material->Render(commandList);

        m_Meshes[i]->OnRender(commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
}

bool Object3DComponent::IsInitialized()
{
    return m_Initialized;
}

void Object3DComponent::Unload()
{
    for (auto mesh : m_Meshes)
    {
        mesh->Release();
    }
}

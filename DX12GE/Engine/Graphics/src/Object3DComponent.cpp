#include "../Object3DComponent.h"
#include "../../Base/DX12LibPCH.h"
#include "../../Base/Singleton.h"

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
        if (!m_Meshes[i]->m_Material->CanDrawIt()) continue;

        if (Singleton::GetCurrentPass()->Get() != CurrentPass::Shadow && Singleton::GetCurrentPass()->Get() != CurrentPass::Lighting)
            m_Meshes[i]->m_Material->Render(commandList);

        m_Meshes[i]->OnRender(commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
}

bool Object3DComponent::IsInitialized()
{
    return m_Initialized;
}

void Object3DComponent::Destroy()
{
    if (!m_Initialized) return;
    m_Initialized = false;

    for (auto mesh : m_Meshes)
    {
        mesh->Destroy();
    }
}

#include "../Object3DComponent.h"
#include "../../Base/DX12LibPCH.h"
#include "../../Base/Singleton.h"

void Object3DComponent::OnLoad(std::vector<Mesh3DComponent*>& meshes, std::vector<Vector3>* modelVertices)
{
    m_Meshes = meshes;
    m_Vertices = *modelVertices;
    
    bool init = true;
    for (auto mesh : m_Meshes)
    {
        init &= mesh->IsInitialized();
    }
    m_Initialized = init;
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

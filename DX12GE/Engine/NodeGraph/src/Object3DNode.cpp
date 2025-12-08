#include "../../Graphics/AssimpModelLoader.h"
#include "../../Graphics/ResourceStorage.h"
#include "../../Base/Singleton.h"

Object3DNode::Object3DNode() : Node3D()
{
    Rename("Object3DNode");
}

bool Object3DNode::Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
    AssimpModelLoader modelLoader;
    float yOffset;
    uint32_t id = modelLoader.LoadModelData(commandList, filePath, yOffset);
    if (id == -1) return false;
    SetComponentId(id);
    Transform.SetDefault(yOffset);
    OnLoad();
    return true;
}

void Object3DNode::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix)
{
    if (m_ComponentId == -1) return;

    XMMATRIX wvp = GetWorldMatrix();
    XMMATRIX mvp = XMMatrixMultiply(wvp, viewProjMatrix);

    if (Singleton::GetCurrentPass()->Get() == CurrentPass::Shadow)
    {
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvp, 0);
    }
    else if (Singleton::GetCurrentPass()->Get() == CurrentPass::Geometry)
    {
        ShaderResources::GetObjectCB()->WorldViewProjection = wvp;
        ShaderResources::GetObjectCB()->ModelViewProjection = mvp;
        ShaderResources::SetGraphicsObjectCB(commandList, 0);
    }
    else {}

    ResourceStorage::GetObject3D(m_ComponentId)->OnRender(commandList);
}

void Object3DNode::Destroy(bool keepComponent)
{
    uint32_t id = m_ComponentId;
    Node3D::Destroy(keepComponent);
    if (keepComponent || TreeHasObjects3DWithComponentId(id)) return;
    ResourceStorage::DeleteObject3DComponentForever(id);
}

void Object3DNode::SetComponentId(uint32_t newId)
{
    if (newId < 0 || newId >= ResourceStorage::ObjectsCount())
    {
        printf("Ошибка: Id компонента 3Д объекта за пределами размера массива в ResourceStorage\n");
        return;
    }
    Node3D::SetComponentId(newId);
}

const std::string Object3DNode::GetObjectFilePath()
{
    if (!IsValid()) return "";
    return ResourceStorage::GetObject3D(m_ComponentId)->ResourcePath;
}

const CollisionBox& Object3DNode::GetCollisionBox()
{
    if (!IsValid())
    {
        throw;
    }
    return ResourceStorage::GetObject3D(m_ComponentId)->Box;
}

bool Object3DNode::TreeHasObjects3DWithComponentId(uint32_t id, Node3D* current)
{
    current = current == nullptr ? Singleton::GetNodeGraph()->GetRoot() : current;

    if (dynamic_cast<Object3DNode*>(current) && current->GetComponentId() == id && id != -1)
    {
        return true;
    }

    for (auto child : current->GetChildren())
    {
        if (TreeHasObjects3DWithComponentId(id, child))
        {
            return true;
        }
    }

    return false;
}

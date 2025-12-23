#include "../../Graphics/AssimpModelLoader.h"
#include "../../Graphics/ResourceStorage.h"
#include "../../Base/Singleton.h"

Object3DNode::Object3DNode() : Node3D(), m_ComponentId(-1)
{
    m_Type = NODE_TYPE_OBJECT3D;
    IsVisible = false;
    Rename("Object3DNode");
}

bool Object3DNode::Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
    AssimpModelLoader modelLoader;
    float yOffset = 0.0f;
    uint32_t id = modelLoader.LoadModelData(commandList, filePath, yOffset);
    Transform.SetDefault(yOffset);
    if (id == -1) return false;
    SetComponentId(id);
    IsVisible = true;
    return true;
}

void Object3DNode::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix)
{
    if (m_ComponentId == -1 || !IsVisible) return;

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
    if (!(keepComponent || TreeHasObjects3DWithComponentId(m_ComponentId)))
    {
        ResourceStorage::DeleteObject3DComponentForever(m_ComponentId);
    }
    m_ComponentId = -1;
    Node3D::Destroy(keepComponent);
}

void Object3DNode::SetComponentId(uint32_t newId)
{
    if (newId < 0 || newId >= ResourceStorage::ObjectsCount())
    {
        printf("Ошибка: Id компонента 3Д объекта за пределами размера массива в ResourceStorage\n");
        return;
    }
    m_ComponentId = newId;
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

void Object3DNode::Clone(Node3D* cloneNode, Node3D* newParrent, bool cloneChildrenRecursive)
{
    if (!cloneNode)
    {
        cloneNode = new Object3DNode();
    }

    Node3D::Clone(cloneNode, newParrent, cloneChildrenRecursive);

    if (cloneNode)
    {
        Object3DNode* obj3D = dynamic_cast<Object3DNode*>(cloneNode);
        obj3D->m_ComponentId = m_ComponentId;
    }
}

void Object3DNode::DrawDebug()
{
    Node3D::DrawDebug();
    Singleton::GetDebugRender()->DrawBoundingBox(GetCollisionBox(), GetWorldMatrix());
}

void Object3DNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);

    j["file_path"] = GetObjectFilePath();
    j["is_visible"] = IsVisible;
}

void Object3DNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
    Node3D::LoadFromJsonData(nodeData);
    IsVisible = nodeData.isVisible;
}

bool Object3DNode::TreeHasObjects3DWithComponentId(uint32_t id, Node3D* current)
{
    if (id == -1) return false;

    current = current == nullptr ? Singleton::GetNodeGraph()->GetRoot() : current;

    if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(current))
    {
        if (obj3D->GetComponentId() == id)
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


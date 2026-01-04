#include "../NodeGraph/PhysicalObjectNode.h"
#include "../../Graphics/AssimpModelLoader.h"
#include "../../Base/Singleton.h"

PhysicalObjectNode::PhysicalObjectNode() : Object3DNode()
{    
    ModelVertices = new std::vector<Vector3>();
    DebugCollisionGeometry = new std::vector<float>();
}

bool PhysicalObjectNode::Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
    AssimpModelLoader modelLoader;
    float yOffset = 0.0f;
    
    uint32_t id = modelLoader.LoadModelData(commandList, filePath, yOffset, ModelVertices);
    Transform.SetDefault(yOffset);
    if (id == -1) return false;
    SetComponentId(id);
    IsVisible = true;
    return true;
}

void PhysicalObjectNode::OnUpdate(const double& deltaTime)
{
    Object3DNode::OnUpdate(deltaTime);
}

void PhysicalObjectNode::Destroy(bool keepComponent)
{
    Object3DNode::Destroy(keepComponent);
}

bool PhysicalObjectNode::AddChild(Node3D* node)
{
    return Object3DNode::AddChild(node);
}

Node3D* PhysicalObjectNode::Clone(Node3D* newParrent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
    return Object3DNode::Clone(newParrent, cloneChildrenRecursive, cloneNode);
}

void PhysicalObjectNode::CreateJsonData(json& j)
{
    Object3DNode::CreateJsonData(j);
    
    j["collision_type"] = collisionType;
    
    j["mass"] = mass;
    
    j["gravity_scale"] = gravityScale;
    
    j["friction_scale"] = frictionScale;
}

void PhysicalObjectNode::SetCurrent()
{
    Object3DNode::SetCurrent();
}

void PhysicalObjectNode::DrawDebug()
{
    Object3DNode::DrawDebug();
    
    for (int i = 0; i < DebugCollisionGeometry->size(); i += 9)
    {
        Vector3 P0(Vector3((*DebugCollisionGeometry)[i], (*DebugCollisionGeometry)[i + 1], (*DebugCollisionGeometry)[i + 2]));
        Vector3 P1(Vector3((*DebugCollisionGeometry)[i + 3], (*DebugCollisionGeometry)[i + 4], (*DebugCollisionGeometry)[i + 5]));
        Vector3 P2(Vector3((*DebugCollisionGeometry)[i + 6], (*DebugCollisionGeometry)[i + 7], (*DebugCollisionGeometry)[i + 8]));
            
        Singleton::GetDebugRender()->DrawTriangle(P0, P1, P2, SimpleMath::Color(0.f, 1.f, 0.f, 1.f));
    }
}

void PhysicalObjectNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
    Object3DNode::LoadFromJsonData(nodeData);

    if (nodeData.collisionType >= 0)
    {
        collisionType = nodeData.collisionType;
    }
    
    if (nodeData.gravityScale >= -1.f)
    {
        gravityScale = nodeData.gravityScale;
    }
    
    if (nodeData.mass >= 0.f)
    {
        mass = nodeData.mass;
    }
    
    if (nodeData.frictionScale >= 0.f)
    {
        frictionScale = nodeData.frictionScale;
    }
}

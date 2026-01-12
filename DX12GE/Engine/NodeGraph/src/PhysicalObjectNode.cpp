#include "../../Graphics/AssimpModelLoader.h"
#include "../Graphics/ResourceStorage.h"
#include "../../Base/Singleton.h"

PhysicalObjectNode::PhysicalObjectNode() : Object3DNode()
{    
	m_Type = NODE_TYPE_PHYSICAL_OBJECT3D;
    Rename("PhysicalObjectNode");
}

bool PhysicalObjectNode::Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
    return Object3DNode::Create(commandList, filePath);
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

Node3D* PhysicalObjectNode::Clone(Node3D* newparent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
    return Object3DNode::Clone(newparent, cloneChildrenRecursive, cloneNode);
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
    
    std::vector<Vector3>* ModelVertices = ResourceStorage::GetObject3D(m_ComponentId)->GetVertices();
    
    for (int i = 0; i < ModelVertices->size(); i += 3)
    {
        //5% offset for better visibility
        Vector3 P0(Vector3::Transform((*ModelVertices)[i] * 1.05f, GetWorldMatrix()));
        Vector3 P1(Vector3::Transform((*ModelVertices)[i + 1] * 1.05f, GetWorldMatrix()));
        Vector3 P2(Vector3::Transform((*ModelVertices)[i + 2] * 1.05f, GetWorldMatrix()));
            
        Singleton::GetDebugRender()->DrawTriangle(P0, P1, P2, SimpleMath::Color(0.f, 1.f, 0.f, 1.f));
    }
}

std::vector<Vector3>* PhysicalObjectNode::GetVertices()
{
    return ResourceStorage::GetObject3D(m_ComponentId)->GetVertices();
}

void PhysicalObjectNode::SetCollisionGeometry(std::vector<Vector3>* vertices)
{
    ResourceStorage::GetObject3D(m_ComponentId)->GetVertices()->swap(*vertices);
    vertices->clear();
};

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

#include "../NodeGraph/PhysicalObjectNode.h"
#include "../../Graphics/AssimpModelLoader.h"

PhysicalObjectNode::PhysicalObjectNode() : Object3DNode()
{    
    ModelVertices = new std::vector<float>();
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
}

void PhysicalObjectNode::SetCurrent()
{
    Object3DNode::SetCurrent();
}

void PhysicalObjectNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
    Object3DNode::LoadFromJsonData(nodeData);
    
    collisionType = nodeData.collisionType;
    gravityScale = nodeData.gravityScale;
    mass = nodeData.mass;
    frictionScale = nodeData.frictionScale;
}

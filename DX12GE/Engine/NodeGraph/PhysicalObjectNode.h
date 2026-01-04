#pragma once
#include "Object3DNode.h"
#include "../Physics/PhysicsDataTypes.h"

class PhysicalObjectNode: public Object3DNode
{
protected:
	std::vector<float>* ModelVertices;
    
    CollisionTypeEnum collisionType = COLLISION_TYPE_STATIC;
        
    float gravityScale = 1.0f;
        
    float mass = 1.0f;
        
    float frictionScale = 1.0f;
    
public:
    PhysicalObjectNode();

    virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath) override;
    
    virtual void OnUpdate(const double& deltaTime) override;

    virtual void Destroy(bool keepComponent = true) override;

    virtual bool AddChild(Node3D* node) override;

    virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

    virtual void CreateJsonData(json& j) override;

    virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

    virtual void SetCurrent() override;
    
    const std::vector<float>& GetVertices() { return *ModelVertices; };
    
    const CollisionTypeEnum& GetCollisionType() { return collisionType; }
    
    const float& GetGravityScale() { return gravityScale; }
    
    const float& GetMass() { return mass; }
    
    const float& GetFrictionScale() { return frictionScale; }
};

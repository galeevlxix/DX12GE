#pragma once
#include "Object3DNode.h"
#include "../Physics/PhysicsDataTypes.h"

class PhysicalObjectNode: public Object3DNode
{
protected:
	std::vector<Vector3>* ModelVertices;
            
    CollisionTypeEnum collisionType = COLLISION_TYPE_STATIC_MESH;
        
    float gravityScale = 1.0f;
        
    float mass = 1.0f;
        
    float frictionScale = 0.2f;
    
public:
    PhysicalObjectNode();

    virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath) override;
    
    virtual void OnUpdate(const double& deltaTime) override;

    virtual void Destroy(bool keepComponent = true) override;

    virtual bool AddChild(Node3D* node) override;

    virtual Node3D* Clone(Node3D* newparent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

    virtual void CreateJsonData(json& j) override;

    virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

    virtual void SetCurrent() override;
    
	virtual void DrawDebug() override;
    
    std::vector<Vector3>* GetVertices() { return ModelVertices; };
    
    void SetCollisionGeometry(std::vector<Vector3>* vertices);
    
    const CollisionTypeEnum& GetCollisionType() { return collisionType; }
    
    const float& GetGravityScale() { return gravityScale; }
    
    const float& GetMass() { return mass; }
    
    const float& GetFrictionScale() { return frictionScale; }
};

#pragma once
#include "Object3DNode.h"

class PhysicalObjectNode: public Object3DNode
{
public:
    PhysicalObjectNode();

    virtual void OnUpdate(const double& deltaTime) override;

    virtual void Destroy(bool keepComponent = true) override;

    virtual bool AddChild(Node3D* node) override;

    virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

    virtual void CreateJsonData(json& j) override;

    virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

    virtual void SetCurrent() override;
};

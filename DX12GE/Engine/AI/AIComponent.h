#pragma once
#include <DirectXMath.h>
#include "BehaviorTree.h"

class Object3DEntity;

class AIComponent {
    std::unique_ptr<BehaviorTree> tree;
public:
    explicit AIComponent(BehaviorTree* t) : tree(t) {}
    void Update(float dt, Object3DEntity* owner) {
        if (tree) tree->tick(dt, owner);
    }
};
#pragma once
#include "Behavior.h"

class BehaviorTree {
    BehaviorPtr root;
public:
    explicit BehaviorTree(BehaviorPtr r) : root(std::move(r)) {}
    void tick(float dt, Object3DEntity* owner) {
        if (root) root->tick(dt, owner);
    }
};
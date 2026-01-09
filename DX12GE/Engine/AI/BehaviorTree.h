#pragma once
#include "Behavior.h"

class BehaviorTree {
    BehaviorPtr root;
    std::unique_ptr<Blackboard> blackboard;
public:
    explicit BehaviorTree(BehaviorPtr r, Blackboard* bb = new Blackboard()) : root(std::move(r)), blackboard(bb) {}
    Blackboard& GetBlackboard() { return *blackboard; }
    BehaviorTree* Clone() const {
        return new BehaviorTree(root->Clone());
    }
    void tick(float dt, Object3DNode* owner) {
        if (root) root->tick(dt, owner, *blackboard);
    }
};
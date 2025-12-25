#pragma once
#include "../NodeGraph/Node3D.h"
#include "../NodeGraph/Object3DNode.h"
#include "../AI/BehaviorTree.h"

class AINode : public Node3D {
public:
    AINode() {
        m_Type = NODE_TYPE_AI;
        m_Name = "AINode";
    }

    // Explicit constructor to take ownership of a tree
    explicit AINode(std::unique_ptr<BehaviorTree> tree) : m_BehaviorTree(std::move(tree)) {
        m_Type = NODE_TYPE_AI;
        m_Name = "AINode";
    }

    void OnUpdate(const double& deltaTime) override {
        // Cast parent to Object3DNode because our Behaviors expect it
        if (m_Parrent && m_BehaviorTree) {
            Object3DNode* owner = dynamic_cast<Object3DNode*>(m_Parrent);
            if (owner) {
                m_BehaviorTree->tick(static_cast<float>(deltaTime), owner);
            }
        }
        Node3D::OnUpdate(deltaTime); // Update children if any
    }

    void SetBehaviorTree(std::unique_ptr<BehaviorTree> tree) {
        m_BehaviorTree = std::move(tree);
    }

protected:
    std::unique_ptr<BehaviorTree> m_BehaviorTree;
};

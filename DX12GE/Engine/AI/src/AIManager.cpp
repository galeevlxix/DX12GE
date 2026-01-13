#include "../AIManager.h"
#include "../NodeGraph/Object3DNode.h"
#include "../NodeGraph/NodeTypeEnum.h"

AINode* AIManager::AttachAI(Object3DNode* object, std::unique_ptr<BehaviorTree> tree) {
    if (!object) return nullptr;

    AINode* aiNode = new AINode(std::move(tree));
    object->AddChild(aiNode);
    return aiNode;
}

AINode* AIManager::GetAI(Object3DNode* object) {
    if (!object) return nullptr;
    
    for (auto* child : object->GetChildren()) {
        if (child->GetType() == NODE_TYPE_AI) {
            return dynamic_cast<AINode*>(child);
        }
    }
    return nullptr;
}

void AIManager::SetAIEnabled(Object3DNode* object, bool enabled) {
    if (AINode* ai = GetAI(object)) {
        ai->SetEnabled(enabled);
    }
}

void AIManager::RemoveAI(Object3DNode* object) {
    if (AINode* ai = GetAI(object)) {
        ai->Destroy(); 
    }
}

#pragma once
#include "../NodeGraph/Object3DNode.h"
#include "../NodeGraph/AINode.h"
#include "BehaviorTree.h"
#include <memory>
#include <vector>

class AIManager {
public:
    static AIManager& Get() {
        static AIManager instance;
        return instance;
    }

    // Attach a built BT to an object (creates AINode)
    AINode* AttachAI(Object3DNode* object, std::unique_ptr<BehaviorTree> tree);

    // Toggle AI On/Off
    void SetAIEnabled(Object3DNode* object, bool enabled);
    
    // Remove AI from object
    void RemoveAI(Object3DNode* object);

    // Get AINode from object
    AINode* GetAI(Object3DNode* object);

private:
    AIManager() = default;
};

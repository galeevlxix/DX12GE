#pragma once
//===============================================================================================================
//          THIS FILE IS NOT FOR COMPILATION IN ANY WAY, SHAPE OR FORM. IT ONLY CONTAINS
//          THE EXAMPLES THAT YOU COULD INSERT INTO SINLGEGPUGAME.LOADCONTENT TO TEST OUT
//          THE AIS AND WHATNOT. THE EXAMPLES ARE NOT ROBUST AND MAY NOT WORK IF THE
//          OBJECTS IN THE EXAMPLE ARE REMOVED FROM THE SCENE. OR THEY MAY JUST BREAK, IDGAF
//=================================================================================================================

/*
// AI SETUP - KNIGHT AI CHASES PLAYER, PLAYER RUNS AWAY. SIMPLE AS THAT
    {
        // 1. Get Entities
        auto graph = Singleton::GetNodeGraph();
        Object3DNode* player = dynamic_cast<Object3DNode*>(graph->GetNodeByPath("root/tp_player")); 
        Object3DNode* car = dynamic_cast<Object3DNode*>(graph->GetNodeByPath("root/gas1/knight")); // Assuming 'ai_car' exists
    
        if (player && car) {
            std::cout << "Building tree";
            // 2. Build Tree
            BehaviorTreeBuilder builder;
            auto treePtr = builder
                .selector()
                    .sequence()
                        .action(new MoveToTarget(player, 5.f, 1.f))
                    .end()
                    .action(new RandomPointMove(50.f))
                .end()
                .build();
    
            BehaviorTreeBuilder builder2;
            auto treePtr2 = builder2
                .selector()
                    .sequence()
                        .action(new MoveAwayFromTarget(car, 5.f, 50.f))
                    .end()
                    .action(new RandomPointMove(50.f))
                .end()
                .build();
            
            std::unique_ptr<BehaviorTree> kTree = std::make_unique<BehaviorTree>(std::move(treePtr));
            std::unique_ptr<BehaviorTree> pTree = std::make_unique<BehaviorTree>(std::move(treePtr2));
            
            AINode* aiNode = new AINode(std::move(kTree));
            AINode* aiNode2 = new AINode(std::move(pTree));
            aiNode->Rename("AI_Controller");
    
            // 4. Attach AINode to Car
            car->AddChild(aiNode);
            player->AddChild(aiNode2);
        }
    }
*/

/*
    // AI SETUP - UTILITY DEMONSTATION. KNIGHT PICKS ACTION BASED ON UTILITY. KNIGHT'S CLONE IS WATCHING 
    // 1. Get Entities
    auto graph = Singleton::GetNodeGraph();
    Object3DNode* car1 = dynamic_cast<Object3DNode*>(graph->GetNodeByPath("root/gas1/knight")); 
    
    // Clone car to make a second player
    Object3DNode* car2 = nullptr;
    if (car1) {
        Node3D* clone = car1->Clone(car1->GetParrent(), true);
        clone->Rename("knight_2");
        car2 = dynamic_cast<Object3DNode*>(clone);
        car2->Transform.SetPosition(Vector3(10, 0, 10)); // Offset it
    }

    if (car1 && car2) {
         // UTILITY AI EXAMPLE BUILDER
        auto BuildUtilityTree = [](Object3DNode* me, Object3DNode* other) {
            BehaviorTreeBuilder builder;
            return builder
                .utilitySelector()
                    // Option 1: Chase Player (Utility 0.3)
                    .utilityDecorator(Aggregation::Average)
                        .score(new ConstantFactor(0.3f))
                        .sequence()
                            .action(new MoveToTarget(other, 8.0f, 2.0f))
                        .end()
                    .end()
                    // Option 2: Wandering (Utility 0.8) -> SHOULD PICK THIS
                    .utilityDecorator(Aggregation::Average)
                        .score(new ConstantFactor(0.8f))
                        .sequence()
                            .action(new RandomPointMove(30.0f, 10.0f))
                        .end()
                    .end()
                .end()
                .build();
        };

        // AI 1 Setup (Utility AI)
        {
            Blackboard* bb = new Blackboard();
            auto treePtr = BuildUtilityTree(car1, car2); // car2 is dummy target
            std::unique_ptr<BehaviorTree> bTree = std::make_unique<BehaviorTree>(std::move(treePtr), bb);

            AINode* aiNode = new AINode(std::move(bTree));
            aiNode->Rename("AI_Utility_Controller");
            car1->AddChild(aiNode);
        }

        // AI 2 Setup (Simple MoveToTarget for visual reference)
        {
             BehaviorTreeBuilder builder;
             auto treePtr = builder
                .selector()
                    .action(new MoveToTarget(car1, 0.0f, 0.0f)) // Stay put or follow slowly
                .end()
                .build();
            std::unique_ptr<BehaviorTree> bTree = std::make_unique<BehaviorTree>(std::move(treePtr));
            
            AINode* aiNode = new AINode(std::move(bTree));
            aiNode->Rename("AI_Observer");
            car2->AddChild(aiNode);
        }
    }
*/

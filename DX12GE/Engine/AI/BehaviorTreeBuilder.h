// BehaviorTreeBuilder.h
#pragma once
#include "Behavior.h"
#include "Composites.h"
#include <stack>
#include <vector>
#include <memory>

struct BuilderContext {
    std::vector<BehaviorPtr> children;
    bool isSequence = false;
};

class BehaviorTreeBuilder {
    std::stack<BuilderContext> stack;

public:
    BehaviorTreeBuilder() { stack.emplace(); }

    BehaviorTreeBuilder& sequence();
    BehaviorTreeBuilder& selector();
    BehaviorTreeBuilder& action(Behavior* b);
    BehaviorTreeBuilder& end();

    BehaviorPtr build();
};
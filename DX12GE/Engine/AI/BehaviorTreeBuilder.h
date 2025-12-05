#pragma once
#include "Behavior.h"
#include "Composites.h"
#include <stack>
#include <vector>
#include <memory>

enum class NodeType { Sequence, Selector, ActiveSelector, Parallel, Monitor, Repeat, Invert, UntilFail };

struct BuilderContext {
    std::vector<BehaviorPtr> children;
    NodeType type = NodeType::Selector;
    Policy successPolicy = Policy::RequireAll;
    Policy failurePolicy = Policy::RequireOne;
    int decoratorLimit = 2;

    BuilderContext() = default;
    BuilderContext(BuilderContext&& other) noexcept
        : children(std::move(other.children)),
          type(other.type),
          successPolicy(other.successPolicy),
          failurePolicy(other.failurePolicy) {}

    BuilderContext& operator=(BuilderContext&& other) noexcept {
        if (this != &other) {
            children = std::move(other.children);
            type = other.type;
            successPolicy = other.successPolicy;
            failurePolicy = other.failurePolicy;
        }
        return *this;
    }

    BuilderContext(const BuilderContext&) = delete;
    BuilderContext& operator=(const BuilderContext&) = delete;
};

class BehaviorTreeBuilder {
    std::stack<BuilderContext> stack;

public:
    BehaviorTreeBuilder() { stack.emplace(); }  // Dummy root context

    BehaviorTreeBuilder& sequence();
    BehaviorTreeBuilder& selector();
    BehaviorTreeBuilder& activeSelector();
    BehaviorTreeBuilder& parallel(Policy success = Policy::RequireAll, Policy failure = Policy::RequireOne);
    BehaviorTreeBuilder& monitor();
    BehaviorTreeBuilder& action(Behavior* b);
    BehaviorTreeBuilder& condition(Condition* c);
    BehaviorTreeBuilder& repeat(int limit);
    BehaviorTreeBuilder& invert();
    BehaviorTreeBuilder& untilFail();
    BehaviorTreeBuilder& end();

    BehaviorPtr build();
};
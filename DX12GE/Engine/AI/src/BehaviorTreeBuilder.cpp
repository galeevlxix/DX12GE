#include "../BehaviorTreeBuilder.h"
#include "../Composites.h"

BehaviorTreeBuilder& BehaviorTreeBuilder::sequence() {
    BuilderContext ctx;
    ctx.type = NodeType::Sequence;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::selector() {
    BuilderContext ctx;
    ctx.type = NodeType::Selector;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::activeSelector() {
    BuilderContext ctx;
    ctx.type = NodeType::ActiveSelector;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::parallel(Policy success, Policy failure) {
    BuilderContext ctx;
    ctx.type = NodeType::Parallel;
    ctx.successPolicy = success;
    ctx.failurePolicy = failure;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::monitor() {
    BuilderContext ctx;
    ctx.type = NodeType::Monitor;
    ctx.successPolicy = Policy::RequireAll;
    ctx.failurePolicy = Policy::RequireOne;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::action(Behavior* b) {
    if (stack.empty()) {
        stack.emplace();  // Ensure a context if stack is empty
    }
    stack.top().children.emplace_back(b);
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::condition(Condition* c) {
    stack.top().children.emplace_back(c);  // Treat as Behavior
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::end() {
    if (stack.size() <= 1) return *this;

    BuilderContext current = std::move(stack.top());
    stack.pop();

    BehaviorPtr nodePtr;
    switch (current.type) {
    case NodeType::Sequence:
        nodePtr = std::make_unique<Sequence>();
        break;
    case NodeType::Selector:
        nodePtr = std::make_unique<Selector>();
        break;
    case NodeType::ActiveSelector:
        nodePtr = std::make_unique<ActiveSelector>();
        break;
    case NodeType::Parallel:
        nodePtr = std::make_unique<Parallel>(current.successPolicy, current.failurePolicy);
        break;
    case NodeType::Monitor:
        nodePtr = std::make_unique<Monitor>();
        break;
    }
    
    Composite* node = static_cast<Composite*>(nodePtr.get());
    for (auto& child : current.children) {
        node->addChild(std::move(child));
    }

    stack.top().children.emplace_back(std::move(nodePtr));
    return *this;
}

BehaviorPtr BehaviorTreeBuilder::build() {
    while (stack.size() > 1) {
        end();
    }
    auto& rootChildren = stack.top().children;
    if (rootChildren.empty()) return nullptr;
    return std::move(rootChildren[0]);
}
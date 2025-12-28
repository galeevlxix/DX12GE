#include "../BehaviorTreeBuilder.h"
#include "../Composites.h"
#include "../Decorators.h"

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
    stack.top().children.emplace_back(c);  // Practically same as Behavior, use for visibility
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::repeat(int limit) {
    BuilderContext ctx;
    ctx.type = NodeType::Repeat;
    ctx.decoratorLimit = limit;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::invert() {
    BuilderContext ctx;
    ctx.type = NodeType::Invert;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::untilFail() {
    BuilderContext ctx;
    ctx.type = NodeType::UntilFail;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::utilitySelector() {
    BuilderContext ctx;
    ctx.type = NodeType::UtilitySelector;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::utilityDecorator(Aggregation aggr) {
    BuilderContext ctx;
    ctx.type = NodeType::UtilityDecorator;
    ctx.aggregation = aggr;
    stack.emplace(std::move(ctx));
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::score(UtilityFactor* factor) {
    if (!stack.empty()) {
        stack.top().factors.emplace_back(std::shared_ptr<UtilityFactor>(factor));
    }
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
    case NodeType::Repeat:
        nodePtr = std::make_unique<Repeat>(current.decoratorLimit);
        break;
    case NodeType::Invert:
        nodePtr = std::make_unique<Invert>();
        break;
    case NodeType::UntilFail:
        nodePtr = std::make_unique<UntilFail>();
        break;
    case NodeType::UtilitySelector:
        nodePtr = std::make_unique<UtilitySelector>();
        break;
    case NodeType::UtilityDecorator:
        auto decorator = std::make_unique<UtilityDecorator>(nullptr, current.aggregation);
        for(auto& f : current.factors) {
            decorator->AddFactor(f);
        }
        nodePtr = std::move(decorator);
        break;
    }
    
    if (dynamic_cast<Decorator*>(nodePtr.get())) {
        Decorator* decorator = static_cast<Decorator*>(nodePtr.get());
        if (!current.children.empty()) {
            decorator->setChild(std::move(current.children[0]));
        }
    } else if (dynamic_cast<Composite*>(nodePtr.get())) {
        Composite* composite = static_cast<Composite*>(nodePtr.get());
        for (auto& child : current.children) {
            composite->addChild(std::move(child));
        }
    }
    stack.top().children.emplace_back(std::move(nodePtr));
    return *this;
}

BehaviorPtr BehaviorTreeBuilder::build() {
    while (stack.size() > 1) {
        end();  // Auto-close all open levels
    }
    auto& rootChildren = stack.top().children;
    if (rootChildren.empty()) return nullptr;
    return std::move(rootChildren[0]);  // Return the root BehaviorPtr
}
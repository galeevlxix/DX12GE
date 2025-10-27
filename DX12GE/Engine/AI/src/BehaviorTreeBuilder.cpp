#include "../BehaviorTreeBuilder.h"
#include "../Composites.h"

BehaviorTreeBuilder& BehaviorTreeBuilder::sequence() {
    stack.emplace(BuilderContext{{}, true});
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::selector() {
    stack.emplace(BuilderContext{{}, false});
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::action(Behavior* b) {
    if (stack.empty()) {
        stack.emplace();
    }
    stack.top().children.emplace_back(b);
    return *this;
}

BehaviorTreeBuilder& BehaviorTreeBuilder::end() {
    if (stack.size() <= 1) return *this;

    BuilderContext current = std::move(stack.top());
    stack.pop();

    Composite* node = nullptr;
    if (current.isSequence) {
        node = new Sequence();
    } else {
        node = new Selector();
    }
    for (auto& child : current.children) {
        node->addChild(std::move(child));
    }

    stack.top().children.emplace_back(node);
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
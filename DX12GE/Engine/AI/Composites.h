#pragma once
#include "Behavior.h"

#include <vector>
#include <memory>

class Composite : public Behavior {
protected:
    typedef std::vector<BehaviorPtr> Behaviors;
    Behaviors m_Children;

public:
    virtual void addChild(BehaviorPtr child) {
        m_Children.push_back(std::move(child));
    }

    void removeChild(size_t index) {
        if (index < m_Children.size()) {
            m_Children.erase(m_Children.begin() + index);
        }
    }

    void clearChildren() {
        m_Children.clear();
    }

protected:
    Composite() = default;
};

class Sequence : public Composite {
protected:
    size_t m_CurrentChildIndex = 0;  // Tracks running child across ticks

    virtual void onInitialize() {
        m_CurrentChildIndex = 0;
    }

    virtual Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) {
        while (m_CurrentChildIndex < m_Children.size()) {
            Status s = m_Children[m_CurrentChildIndex]->tick(dt, owner, blackboard);
            if (s != Status::SUCCESS) {
                return s;  // Propagate FAILURE or RUNNING
            }
            m_CurrentChildIndex++;
        }
        return Status::SUCCESS;  // Only if all children succeeded
    }

    virtual void onTerminate(Status status) {
        // Reset for next execution; abort running children
        m_CurrentChildIndex = 0;
        if (status == Status::ABORTED) {
            for (size_t i = m_CurrentChildIndex; i < m_Children.size(); ++i) {
                m_Children[i]->abort();
            }
        }
    }

public:
    Sequence() = default;
    BehaviorPtr Clone() const override {
        auto clone = std::make_unique<Sequence>();
        for (const auto& child : m_Children) {
            clone->addChild(child->Clone());
        }
        return clone;
    }
};


class Selector : public Composite {
protected:
    size_t m_CurrentChildIndex = 0;

    void onInitialize() override {
        m_CurrentChildIndex = 0;
    }

    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override {
        while (m_CurrentChildIndex < m_Children.size()) {
            Status s = m_Children[m_CurrentChildIndex]->tick(dt, owner, blackboard);
            if (s != Status::FAILURE) return s;
            m_CurrentChildIndex++;
        }
        return Status::FAILURE;
    }

    void onTerminate(Status status) override {
        m_CurrentChildIndex = 0;
        if (status == Status::ABORTED) {
            for (size_t i = m_CurrentChildIndex; i < m_Children.size(); ++i) {
                m_Children[i]->abort();
            }
        }
    }

public:
    Selector() = default;
    BehaviorPtr Clone() const override {
        auto clone = std::make_unique<Selector>();
        for (const auto& child : m_Children) {
            clone->addChild(child->Clone());
        }
        return clone;
    }
};

class ActiveSelector : public Selector {
protected:
    size_t m_PreviousChildIndex = 0;  // To track previous for abort

    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override {
        size_t prevIndex = m_CurrentChildIndex;

        // Reset to start from first child each tick
        onInitialize();
        
        Status result = Selector::update(dt, owner, blackboard);

        // If switched to different child, abort the previous
        if (prevIndex != m_CurrentChildIndex && prevIndex < m_Children.size()) {
            m_Children[prevIndex]->abort();
        }

        return result;
    }

public:
    ActiveSelector() = default;
    BehaviorPtr Clone() const override {
        auto clone = std::make_unique<ActiveSelector>();
        for (const auto& child : m_Children) {
            clone->addChild(child->Clone());
        }
        return clone;
    }
};

enum class Policy { RequireOne, RequireAll };

class Parallel : public Composite {
protected:
    Policy m_eSuccessPolicy;
    Policy m_eFailurePolicy;

    void onInitialize() override {}

    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override {
        size_t successCount = 0, failureCount = 0;
        size_t size = m_Children.size();
        for (size_t i = 0; i < size; ++i) {
            Behavior* b = m_Children[i].get();
            if (b->m_Status != Status::SUCCESS && b->m_Status != Status::FAILURE) {
                b->tick(dt, owner, blackboard);
            }
            if (b->m_Status == Status::SUCCESS) {
                ++successCount;
                if (m_eSuccessPolicy == Policy::RequireOne) return Status::SUCCESS;
            }
            if (b->m_Status == Status::FAILURE) {
                ++failureCount;
                if (m_eFailurePolicy == Policy::RequireOne) return Status::FAILURE;
            }
        }
        if (m_eFailurePolicy == Policy::RequireAll && failureCount == size) return Status::FAILURE;
        if (m_eSuccessPolicy == Policy::RequireAll && successCount == size) return Status::SUCCESS;
        return Status::RUNNING;
    }

    void onTerminate(Status status) override {
        for (auto& child : m_Children) {
            if (child->m_Status == Status::RUNNING) {
                child->abort();
            }
        }
    }

public:
    Parallel(Policy success, Policy failure)
        : m_eSuccessPolicy(success), m_eFailurePolicy(failure) {}
    
    BehaviorPtr Clone() const override {
        auto clone = std::make_unique<Parallel>(m_eSuccessPolicy, m_eFailurePolicy);
        for (const auto& child : m_Children) {
            clone->addChild(child->Clone());
        }
        return clone;
    }
};

class Monitor : public Parallel {
public:
    Monitor() : Parallel(Policy::RequireAll, Policy::RequireOne) {}

    BehaviorPtr Clone() const override {
        auto clone = std::make_unique<Monitor>();
        for (const auto& child : m_Children) {
            clone->addChild(child->Clone());
        }
        clone->numConditions = numConditions;
        return clone;
    }

    int numConditions = 0;

    void addChild(BehaviorPtr child) override {
        if (dynamic_cast<Condition*>(child.get())) {
            m_Children.insert(m_Children.begin(), std::move(child));
            numConditions++;
        } else {
            m_Children.emplace_back(std::move(child));
        }
    }

    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override {
        size_t successCount = 0, failureCount = 0;
        size_t size = m_Children.size();
        for (size_t i = 0; i < size; ++i) {
            Behavior* b = m_Children[i].get();
            if ((b->m_Status != Status::SUCCESS && b->m_Status != Status::FAILURE) || i < numConditions) {
                b->tick(dt, owner, blackboard);
            }
            if (b->m_Status == Status::SUCCESS) {
                ++successCount;
                if (m_eSuccessPolicy == Policy::RequireOne) return Status::SUCCESS;
            }
            if (b->m_Status == Status::FAILURE) {
                ++failureCount;
                if (m_eFailurePolicy == Policy::RequireOne) return Status::FAILURE;
            }
        }
        if (m_eFailurePolicy == Policy::RequireAll && failureCount == size) return Status::FAILURE;
        if (m_eSuccessPolicy == Policy::RequireAll && successCount == size) return Status::SUCCESS;
        return Status::RUNNING;
    }
};
#pragma once
#include "Behavior.h"

#include <vector>
#include <memory>

class Composite : public Behavior {
protected:
    typedef std::vector<BehaviorPtr> Behaviors;
    Behaviors m_Children;

public:
    void addChild(BehaviorPtr child) {
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
    size_t m_CurrentChildIndex = 0;

    virtual void onInitialize() {
        m_CurrentChildIndex = 0;
    }

    virtual Status update(float dt, Object3DEntity* owner) {
        while (m_CurrentChildIndex < m_Children.size()) {
            Status s = m_Children[m_CurrentChildIndex]->tick(dt, owner);
            if (s != Status::SUCCESS) {
                return s;
            }
            m_CurrentChildIndex++;
        }
        return Status::SUCCESS;
    }

    virtual void onTerminate(Status status) {
        m_CurrentChildIndex = 0;
        if (status == Status::ABORTED) {
            for (size_t i = m_CurrentChildIndex; i < m_Children.size(); ++i) {
                m_Children[i]->abort();
            }
        }
    }

public:
    Sequence() = default;
};


class Selector : public Composite {
protected:
    size_t m_CurrentChildIndex = 0;

    void onInitialize() override {
        m_CurrentChildIndex = 0;
    }

    Status update(float dt, Object3DEntity* owner) override {
        while (m_CurrentChildIndex < m_Children.size()) {
            Status s = m_Children[m_CurrentChildIndex]->tick(dt, owner);
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
};

class ActiveSelector : public Selector {
protected:
    size_t m_PreviousChildIndex = 0;

    Status update(float dt, Object3DEntity* owner) override {
        size_t prevIndex = m_CurrentChildIndex;
        
        onInitialize();
        
        Status result = Selector::update(dt, owner);
        
        if (prevIndex != m_CurrentChildIndex && prevIndex < m_Children.size()) {
            m_Children[prevIndex]->abort();
        }

        return result;
    }

public:
    ActiveSelector() = default;
};

enum class Policy { RequireOne, RequireAll };

class Parallel : public Composite {
protected:
    Policy m_eSuccessPolicy;
    Policy m_eFailurePolicy;

    void onInitialize() override {}

    Status update(float dt, Object3DEntity* owner) override {
        size_t successCount = 0, failureCount = 0;
        size_t size = m_Children.size();
        for (size_t i = 0; i < size; ++i) {
            Behavior* b = m_Children[i].get();
            if (b->m_Status != Status::SUCCESS && b->m_Status != Status::FAILURE) {
                b->tick(dt, owner);
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
};

class Monitor : public Parallel {
public:
    Monitor() : Parallel(Policy::RequireAll, Policy::RequireOne) {}

    void addCondition(Behavior* condition) {
        m_Children.insert(m_Children.begin(), BehaviorPtr(condition));
    }

    void addAction(Behavior* action) {
        m_Children.emplace_back(action);
    }
};
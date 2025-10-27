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
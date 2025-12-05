#pragma once

#include "Behavior.h"

class Decorator : public Behavior {
protected:
    BehaviorPtr m_Child;

public:
    Decorator() = default;
    void setChild(BehaviorPtr child) { m_Child = std::move(child); }

protected:
    void onInitialize() override {}
    void onTerminate(Status status) override {
        if (m_Child && m_Child->m_Status == Status::RUNNING) {
            m_Child->abort();
        }
    }
};

class Repeat : public Decorator {
private:
    int m_Limit;
    int m_Counter = 0;

public:
    Repeat(int limit) : m_Limit(limit) {}
    BehaviorPtr Clone() const override {
        auto clone = std::make_unique<Repeat>(m_Limit);
        if (m_Child) clone->setChild(m_Child->Clone());
        return clone;
    }

protected:
    void onInitialize() override {
        m_Counter = 0;
    }
    void onTerminate(Status status) override {
        m_Counter = 0;
        if (m_Child && m_Child->m_Status == Status::RUNNING) {
            m_Child->abort();
        }
    }

    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override {
        while (true) {
            m_Child->tick(dt, owner, blackboard);
            if (m_Child->m_Status == Status::RUNNING) return Status::RUNNING;
            if (m_Child->m_Status == Status::FAILURE) return Status::FAILURE;
            if (++m_Counter == m_Limit) return Status::SUCCESS;
            m_Child->reset();
        }
    }
};

class Invert : public Decorator {
public:
    BehaviorPtr Clone() const override {
        auto clone = std::make_unique<Invert>();
        if (m_Child) clone->setChild(m_Child->Clone());
        return clone;
    }
protected:
    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override {
        Status s = m_Child->tick(dt, owner, blackboard);
        if (s == Status::SUCCESS) return Status::FAILURE;
        if (s == Status::FAILURE) return Status::SUCCESS;
        return s;
    }
};

class UntilFail : public Decorator {
public:
    BehaviorPtr Clone() const override {
        auto clone = std::make_unique<UntilFail>();
        if (m_Child) clone->setChild(m_Child->Clone());
        return clone;
    }
protected:
    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override {
        Status s = m_Child->tick(dt, owner, blackboard);
        if (s == Status::FAILURE) return Status::SUCCESS;
        if (s == Status::SUCCESS) m_Child->reset();
        return Status::RUNNING;
    }
};
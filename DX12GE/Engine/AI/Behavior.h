#pragma once
#include <memory>
#include <vector>
#include <functional>
#include <DirectXMath.h>
#include "Blackboard.h"

class Object3DNode;

enum class Status { INVALID = -1, SUCCESS, FAILURE, RUNNING, ABORTED };

class Behavior {
public:
    virtual ~Behavior() = default;
    Status tick(float dt, Object3DNode* owner, Blackboard& blackboard) {
        if (m_Status == Status::INVALID) {
            onInitialize();
        }
        m_Status = update(dt, owner, blackboard);
        if (m_Status != Status::RUNNING) {
            onTerminate(m_Status);
        }
        return m_Status;
    }
    virtual void reset() {}
    void abort() {
        onTerminate(Status::ABORTED);
        m_Status = Status::INVALID;
    }
    Status m_Status = Status::INVALID;
    virtual std::unique_ptr<Behavior> Clone() const = 0;
    virtual float getUtility(Object3DNode* owner, Blackboard& blackboard) { return 0.0f; }
protected:
    virtual void onInitialize() {}
    virtual Status update(float dt, Object3DNode* owner, Blackboard& blackboard) = 0;
    virtual void onTerminate(Status status) {}
};

using BehaviorPtr = std::unique_ptr<Behavior>;

class Condition : public Behavior {
protected:
    std::function<bool(Object3DNode*)> m_Check;  // Callback for condition test
    bool m_Negate = false;
    bool m_Monitor = false;

    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override {
        bool result = m_Check(owner);
        if (m_Negate) result = !result;

        if (m_Monitor) {
            return result ? Status::RUNNING : Status::FAILURE;
        } else {
            return result ? Status::SUCCESS : Status::FAILURE;
        }
    }

public:
    Condition(std::function<bool(Object3DNode*)> check, bool negate = false, bool monitor = false)
        : m_Check(check), m_Negate(negate), m_Monitor(monitor) {}

    BehaviorPtr Clone() const override {
        return std::make_unique<Condition>(m_Check, m_Negate, m_Monitor);
    }
};
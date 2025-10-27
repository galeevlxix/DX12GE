#pragma once
#include <memory>
#include <vector>
#include <functional>
#include <DirectXMath.h>

class Object3DEntity;

enum class Status { INVALID = -1, SUCCESS, FAILURE, RUNNING, ABORTED };

class Behavior {
public:
    virtual ~Behavior() = default;
    Status tick(float dt, Object3DEntity* owner) {
        if (m_Status == Status::INVALID) {
            onInitialize();
        }
        m_Status = update(dt, owner);
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
protected:
    Status m_Status = Status::INVALID;

    virtual void onInitialize() {}
    virtual Status update(float dt, Object3DEntity* owner) = 0;
    virtual void onTerminate(Status status) {}
};

using BehaviorPtr = std::unique_ptr<Behavior>;

class Condition : public Behavior {
protected:
    std::function<bool(Object3DEntity*)> m_Check;
    bool m_Negate = false;
    bool m_Monitor = false;

    Status update(float dt, Object3DEntity* owner) override {
        bool result = m_Check(owner);
        if (m_Negate) result = !result;

        if (m_Monitor) {
            return result ? Status::RUNNING : Status::FAILURE;
        } else {
            return result ? Status::SUCCESS : Status::FAILURE;
        }
    }

public:
    Condition(std::function<bool(Object3DEntity*)> check, bool negate = false, bool monitor = false)
        : m_Check(check), m_Negate(negate), m_Monitor(monitor) {}
};
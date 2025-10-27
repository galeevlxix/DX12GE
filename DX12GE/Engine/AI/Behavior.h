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
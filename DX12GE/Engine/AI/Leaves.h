#pragma once
#include <iostream>

#include "Behavior.h"
#include "../NodeGraph/Object3DNode.h"
using namespace DirectX::SimpleMath;

class MoveToTarget : public Behavior {
    Object3DNode* target = nullptr;
    float speed = 15.f;
    float stopDist = 5.f;
public:
    MoveToTarget(Object3DNode* t, float s = 7.5f, float stop = 5.f)
        : target(t), speed(s), stopDist(stop) {}
    BehaviorPtr Clone() const override {
        return std::make_unique<MoveToTarget>(target, speed, stopDist);
    }
protected:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override;
};

class MoveAwayFromTarget : public Behavior {
    Object3DNode* target = nullptr;
    float speed = 15.f;
    float safeDistance = 50.f;  // Run until this far away
public:
    MoveAwayFromTarget(Object3DNode* t, float s = 15.f, float dist = 50.f)
        : target(t), speed(s), safeDistance(dist) {}
    
    BehaviorPtr Clone() const override {
        return std::make_unique<MoveAwayFromTarget>(target, speed, safeDistance);
    }
protected:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override;
};

class FleeRandomly : public Behavior {
    Object3DNode* target = nullptr;
    float speed = 15.f;
    float safeDistance = 50.f;
    float jitterStrength = 0.5f; 
public:
    FleeRandomly(Object3DNode* t, float s = 15.f, float dist = 50.f, float jitter = 0.5f)
        : target(t), speed(s), safeDistance(dist), jitterStrength(jitter) {}

    BehaviorPtr Clone() const override {
        return std::make_unique<FleeRandomly>(target, speed, safeDistance, jitterStrength);
    }
protected:
    void onInitialize() override;
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override;
};

class MoveToSpawn : public Behavior {
    float speed = 15.f;
    float stopDist = 5.f;
public:
    MoveToSpawn(float s = 30.f, float stop = 5.f)
        : speed(s), stopDist(stop) {}
    BehaviorPtr Clone() const override {
        return std::make_unique<MoveToSpawn>(speed, stopDist);
    }
protected:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override;
};

class DebugACtion : public Behavior {
public:
    DebugACtion() {}
    BehaviorPtr Clone() const override {
        return std::make_unique<DebugACtion>();
    }
protected:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override;
};

class IsTargetVisible : public Condition {
public:
    Object3DNode* target = nullptr;
    IsTargetVisible(Object3DNode* t, bool negate = false, bool monitor = false)
        : target(t), Condition([this](Object3DNode* owner) {
            if (!target) return false;
            Vector3 to = target->Transform.GetPosition();
            Vector3 from = owner->Transform.GetPosition();
            float dist = (from - to).Length();
            return dist < 50.f;
        }, negate, monitor) {}

    BehaviorPtr Clone() const override {
        return std::make_unique<IsTargetVisible>(target, m_Negate, m_Monitor);
    }
};

class LinkCheckCondition : public Condition {
public:
    std::string key;
    bool expected;

    LinkCheckCondition(std::string k, bool exp, bool negate = false) 
        : key(k), expected(exp), Condition([this, k, exp](Object3DNode* owner) {
             return false; 
        }, negate) 
    {
    }
};

class CheckBlackboardBool : public Behavior {
    std::string key;
    bool expectedValue;
public:
    CheckBlackboardBool(std::string k, bool val) : key(k), expectedValue(val) {}
    
    BehaviorPtr Clone() const override {
        return std::make_unique<CheckBlackboardBool>(key, expectedValue);
    }
protected:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override {
        if (!blackboard.Has(key)) return Status::FAILURE;
        try {
            bool val = blackboard.Get<bool>(key);
            return (val == expectedValue) ? Status::SUCCESS : Status::FAILURE;
        } catch(...) {
            return Status::FAILURE;
        }
    }
};

class SetBlackboardBool : public Behavior {
    std::string key;
    bool value;
public:
    SetBlackboardBool(std::string k, bool v) : key(k), value(v) {}
    
    BehaviorPtr Clone() const override {
        return std::make_unique<SetBlackboardBool>(key, value);
    }
protected:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override {
        blackboard.Set(key, value);
        return Status::SUCCESS;
    }
};

class Wait : public Behavior {
private:
    float m_Duration;
    float m_Elapsed = 0.0f;

public:
    Wait(float seconds) : m_Duration(seconds) {}

protected:
    void onInitialize() override {
        m_Elapsed = 0.0f;
    }

    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override;

    BehaviorPtr Clone() const override {
        return std::make_unique<Wait>(m_Duration);
    }

    void onTerminate(Status status) override
    {
        m_Elapsed = 0.0f;
    }
};

class MoveToPoint : public Behavior {
private:
    Vector3 m_Target;
    float m_Speed = 15.0f;
    float m_StopDist = 5.0f;

public:
    MoveToPoint(Vector3 point, float speed = 15.0f, float stop = 5.0f)
        : m_Target(point), m_Speed(speed), m_StopDist(stop) {}

protected:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override;

    BehaviorPtr Clone() const override {
        return std::make_unique<MoveToPoint>(m_Target, m_Speed, m_StopDist);
    }
};

class RandomPointMove : public Behavior {
private:
    float m_Radius;
    float m_Speed = 15.0f;
    float m_StopDist = 5.0f;
    Vector3 m_RandomPoint;
    bool m_PointGenerated = false;

public:
    RandomPointMove(float radius, float speed = 15.0f, float stop = 5.0f)
        : m_Radius(radius), m_Speed(speed), m_StopDist(stop) {}

protected:
    void onInitialize() override {
        m_PointGenerated = false;
    }

    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override;

    BehaviorPtr Clone() const override {
        return std::make_unique<RandomPointMove>(m_Radius, m_Speed, m_StopDist);
    }

    void onTerminate(Status status) override
    {
        m_PointGenerated = false;
    }
    
};
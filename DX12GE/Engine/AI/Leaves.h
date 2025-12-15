/*#pragma once
#include <iostream>

#include "Behavior.h"

using namespace DirectX::SimpleMath;

class MoveToTarget : public Behavior {
    Object3DEntity* target = nullptr;
    float speed = 15.f;
    float stopDist = 5.f;
public:
    MoveToTarget(Object3DEntity* t, float s = 7.5f, float stop = 5.f)
        : target(t), speed(s), stopDist(stop) {}
    BehaviorPtr Clone() const override {
        return std::make_unique<MoveToTarget>(target, speed, stopDist);
    }
protected:
    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override;
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
    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override;
};

class DebugACtion : public Behavior {
public:
    DebugACtion() {}
    BehaviorPtr Clone() const override {
        return std::make_unique<DebugACtion>();
    }
protected:
    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override;
};

class IsTargetVisible : public Condition {
public:
    Object3DEntity* target = nullptr;
    IsTargetVisible(Object3DEntity* t, bool negate = false, bool monitor = false)
        : target(t), Condition([this](Object3DEntity* owner) {
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

    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override;

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
    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override;

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

    Status update(float dt, Object3DEntity* owner, Blackboard& blackboard) override;

    BehaviorPtr Clone() const override {
        return std::make_unique<RandomPointMove>(m_Radius, m_Speed, m_StopDist);
    }

    void onTerminate(Status status) override
    {
        m_PointGenerated = false;
    }
    
};*/
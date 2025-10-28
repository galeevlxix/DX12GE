#pragma once
#include "Behavior.h"
#include "../Graphics/Object3DEntity.h"
using namespace DirectX::SimpleMath;

class MoveToTarget : public Behavior {
    Object3DEntity* target = nullptr;
    float speed = 15.f;
    float stopDist = 5.f;
public:
    MoveToTarget(Object3DEntity* t, float s = 7.5f, float stop = 5.f)
        : target(t), speed(s), stopDist(stop) {}
protected:
    Status update(float dt, Object3DEntity* owner) override;
};

class MoveToSpawn : public Behavior {
    float speed = 15.f;
    float stopDist = 5.f;
public:
    MoveToSpawn(float s = 30.f, float stop = 5.f)
        : speed(s), stopDist(stop) {}
protected:
    Status update(float dt, Object3DEntity* owner) override;
};

class DebugACtion : public Behavior {
public:
    DebugACtion() {}
protected:
    Status update(float dt, Object3DEntity* owner) override;
};

class IsTargetVisible : public Condition {
public:
    IsTargetVisible(Object3DEntity* t, bool negate = false, bool monitor = false)
        : Condition([this, t](Object3DEntity* owner) {
            
            if (!t) return false;
            Vector3 to = t->Transform.GetPosition();
            Vector3 from = owner->Transform.GetPosition();
            float dist = (from - to).Length();
            return dist < 50.f;
        }, negate, monitor) {}
};

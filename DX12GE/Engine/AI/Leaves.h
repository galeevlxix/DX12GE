#pragma once
#include "Behavior.h"

class MoveToTarget : public Behavior {
    Object3DEntity* target = nullptr;
    float speed = 15.f;
    float stopDist = 5.f;
public:
    MoveToTarget(Object3DEntity* t, float s = 15.f, float stop = 5.f)
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

class IsTargetVisible : public Behavior {
    Object3DEntity* target = nullptr;
public:
    IsTargetVisible(Object3DEntity* t)
        : target(t) {}
protected:
    Status update(float dt, Object3DEntity* owner) override;
};
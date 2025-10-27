#include "../Leaves.h"
#include "../../Graphics/Object3DEntity.h"
using namespace DirectX::SimpleMath;

Status MoveToTarget::update(float dt, Object3DEntity* owner) {
    if (!target) return Status::FAILURE;
    Vector3 to = target->Transform.GetPosition();
    Vector3 from = owner->Transform.GetPosition();
    Vector3 dir = to - from;
    float dist = dir.Length();
    if (dist <= stopDist) return Status::SUCCESS;

    dir.Normalize();
    from += dir * speed * dt;
    owner->Transform.SetPosition(from);
    
    float yaw = atan2f(dir.x, dir.z);
    owner->Transform.SetRotation(0.f, yaw, 0.f);
    return Status::RUNNING;
}

Status MoveToSpawn::update(float dt, Object3DEntity* owner) {
    Vector3 to = Vector3(-50.0f, 7.0f, 50.0f);
    Vector3 from = owner->Transform.GetPosition();
    Vector3 dir = to - from;
    float dist = dir.Length();
    if (dist <= stopDist) return Status::SUCCESS;

    dir.Normalize();
    from += dir * speed * dt;
    owner->Transform.SetPosition(from);
    
    float yaw = atan2f(dir.x, dir.z);
    owner->Transform.SetRotation(0.f, yaw, 0.f);
    return Status::RUNNING;
}

Status IsTargetVisible::update(float dt, Object3DEntity* owner) {
    if (!target) return Status::FAILURE;
    Vector3 to = target->Transform.GetPosition();
    Vector3 from = owner->Transform.GetPosition();
    float dist = (from - to).Length();
    return (dist < 50.f) ? Status::SUCCESS : Status::FAILURE;
}
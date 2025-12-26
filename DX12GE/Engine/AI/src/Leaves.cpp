#include "../Leaves.h"
using namespace DirectX::SimpleMath;

Status MoveToTarget::update(float dt, Object3DNode* owner, Blackboard& blackboard) {
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

Status MoveAwayFromTarget::update(float dt, Object3DNode* owner, Blackboard& blackboard) {
    if (!target) return Status::FAILURE;

    Vector3 to = target->Transform.GetPosition();
    Vector3 from = owner->Transform.GetPosition();
    Vector3 dir = from - to;
    float dist = dir.Length();
    if (dist >= safeDistance) return Status::SUCCESS;

    dir.Normalize();
    Vector3 newWorldPos = from + dir * speed * dt;
    
    dir.Normalize();
    from += dir * speed * dt;
    owner->Transform.SetPosition(from);
    
    float yaw = atan2f(dir.x, dir.z);
    owner->Transform.SetRotation(0.f, yaw, 0.f);
    return Status::RUNNING;
}

Status MoveToSpawn::update(float dt, Object3DNode* owner, Blackboard& blackboard) {
    std::cout << "MoveToSpawn Action Triggered" << std::endl;
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

Status DebugACtion::update(float dt, Object3DNode* owner, Blackboard& blackboard) {
    std::cout << "Debug Action Triggered" << std::endl;
    return Status::SUCCESS;
}

Status Wait::update(float dt, Object3DNode* owner, Blackboard& blackboard)
{
    m_Elapsed += dt;
    if (m_Elapsed >= m_Duration) return Status::SUCCESS;
    return Status::RUNNING;
}

Status MoveToPoint::update(float dt, Object3DNode* owner, Blackboard& blackboard)
{
    Vector3 from = owner->Transform.GetPosition();
    Vector3 dir = m_Target - from;
    float dist = dir.Length();
    if (dist <= m_StopDist) return Status::SUCCESS;

    dir.Normalize();
    from += dir * m_Speed * dt;
    owner->Transform.SetPosition(from);

    float yaw = atan2f(dir.x, dir.z);
    owner->Transform.SetRotation(0.0f, yaw, 0.0f);
    return Status::RUNNING;
}

Status RandomPointMove::update(float dt, Object3DNode* owner, Blackboard& blackboard)
{
    if (!m_PointGenerated) {
        Vector3 center = owner->Transform.GetPosition();
        float angle = static_cast<float>(rand()) / RAND_MAX * 2 * DirectX::XM_PI;
        float dist = static_cast<float>(rand()) / RAND_MAX * m_Radius;
        m_RandomPoint = center + Vector3(dist * sin(angle), 0.0f, dist * cos(angle));
        m_PointGenerated = true;
    }

    Vector3 from = owner->Transform.GetPosition();
    Vector3 dir = m_RandomPoint - from;
    float dist = dir.Length();
    if (dist <= m_StopDist) return Status::SUCCESS;

    dir.Normalize();
    from += dir * m_Speed * dt;
    owner->Transform.SetPosition(from);

    float yaw = atan2f(dir.x, dir.z);
    owner->Transform.SetRotation(0.0f, yaw, 0.0f);
    return Status::RUNNING;
}
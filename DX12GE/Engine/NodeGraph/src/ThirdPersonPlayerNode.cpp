#include "../ThirdPersonPlayerNode.h"
#include "../../Base/InputSystem.h"

ThirdPersonPlayerNode::ThirdPersonPlayerNode() : FirstPersonPlayerNode()
{
    m_Type = NODE_TYPE_THIRD_PERSON_PLAYER;
	Rename("ThirdPersonPlayerNode");
}

void ThirdPersonPlayerNode::OnUpdate(const double& deltaTime)
{
    if (IsCurrent())
    {
        if (m_Camera)
        {
            m_Camera->Transform.SetPosition(
                m_FlyRadius * cos(m_angle_v) * cos(-m_angle_h),
                m_FlyRadius * sin(m_angle_v) + 3.0,
                m_FlyRadius * cos(m_angle_v) * sin(-m_angle_h)
            );
            m_Camera->Transform.LocalLookAt(Vector3(0, 3, 0));
        }

        if (m_PressedInputs.RBC)
        {
            if (m_PressedInputs.Shift)
            {
                m_MovementSpeed = m_MinMovementSpeed;
            }
            else if (m_PressedInputs.Ctrl)
            {
                m_MovementSpeed = m_MaxMovementSpeed;
            }
            else
            {
                m_MovementSpeed = m_NormalMovementSpeed;
            }

            const Matrix& parMat = m_Parrent->GetWorldMatrix();
            Vector3 direction = m_Camera->GetWorldDirection();

            direction = Vector3::Transform(direction, parMat.Invert());
            direction.y = 0;

            Vector3 right = direction.Cross(Vector3::Up);
            right.Normalize();

            float delta = static_cast<float>(deltaTime);
            if (m_PressedInputs.W)
            {
                Transform.Move(direction * m_MovementSpeed * delta);
            }
            if (m_PressedInputs.S)
            {
                Transform.Move(-direction * m_MovementSpeed * delta);
            }
            if (m_PressedInputs.A)
            {
                Transform.Move(-right * m_MovementSpeed * delta);
            }
            if (m_PressedInputs.D)
            {
                Transform.Move(right * m_MovementSpeed * delta);
            }
            if (m_PressedInputs.E)
            {
                Transform.Move(Vector3::Up * m_MovementSpeed * delta);
            }
            if (m_PressedInputs.Q)
            {
                Transform.Move(Vector3::Down * m_MovementSpeed * delta);
            }
        }
    }    

    Object3DNode::OnUpdate(deltaTime);
}

void ThirdPersonPlayerNode::Destroy(bool keepComponent)
{
	m_Camera = nullptr;
	Object3DNode::Destroy(keepComponent);
}

void ThirdPersonPlayerNode::OnMouseWheel(MouseWheelEventArgs& e)
{
    Object3DNode::OnMouseWheel(e);
    m_FlyRadius = std::clamp(m_FlyRadius - e.WheelDelta, 1.3333f, 30.0f);
}

#include "../ThirdPersonPlayerNode.h"
#include "../../Base/InputSystem.h"

ThirdPersonPlayerNode::ThirdPersonPlayerNode() : FirstPersonPlayerNode()
{
    m_Type = NODE_TYPE_THIRD_PERSON_PLAYER;

    m_FlyRadius = 5.0f;
    MinFlyRadius = 2.0f;
    MaxFlyRadius = 30.0f;

	CameraAnchor = Vector3(0.0f, 3.0f, 0.0f);

	Rename("ThirdPersonPlayerNode");
}

void ThirdPersonPlayerNode::OnUpdate(const double& deltaTime)
{
    if (IsCurrent())
    {
        if (m_PressedInputs.RBC)
        {
            if (m_PressedInputs.Shift)
            {
                m_CurrentMovementSpeed = MinMovementSpeed;
            }
            else if (m_PressedInputs.Ctrl)
            {
                m_CurrentMovementSpeed = MaxMovementSpeed;
            }
            else
            {
                m_CurrentMovementSpeed = NormalMovementSpeed;
            }
            
            Vector3 direction = Vector3::Backward;
            if (m_Camera)
            {
                direction = m_Camera->GetWorldDirection();
            }

            const Matrix& parMat = m_Parrent->GetWorldMatrix();
            direction = Vector3::Transform(direction, parMat.Invert());
            direction.y = 0.0f;

            Vector3 right = direction.Cross(Vector3::Up);
            right.Normalize();

            float delta = static_cast<float>(deltaTime);
            if (m_PressedInputs.W)
            {
                Transform.Move(direction * m_CurrentMovementSpeed * delta);
				Transform.RotateToLocalDirection(direction);
            }
            if (m_PressedInputs.S)
            {
                Transform.Move(-direction * m_CurrentMovementSpeed * delta);
                Transform.RotateToLocalDirection(-direction);
            }
            if (m_PressedInputs.A)
            {
                Transform.Move(-right * m_CurrentMovementSpeed * delta);
                Transform.RotateToLocalDirection(-right);
            }
            if (m_PressedInputs.D)
            {
                Transform.Move(right * m_CurrentMovementSpeed * delta);
                Transform.RotateToLocalDirection(right);
            }
            if (m_PressedInputs.E)
            {
                Transform.Move(Vector3::Up * m_CurrentMovementSpeed * delta);
            }
            if (m_PressedInputs.Q)
            {
                Transform.Move(Vector3::Down * m_CurrentMovementSpeed * delta);
            }
        }

        if (m_Camera)
        {
            /*m_angle_h += deltaTime * PI / 8.0f;
            m_angle_v = PI / 4;*/

            Vector3 camLocalPos = CameraAnchor + Vector3(
                m_FlyRadius * cos(m_angle_v) * cos(-m_angle_h),
                m_FlyRadius * sin(m_angle_v),
                m_FlyRadius * cos(m_angle_v) * sin(-m_angle_h));

            const Matrix& localRotMat = Transform.GetLocalRotationMatrix();
            m_Camera->Transform.SetPosition(Vector3::Transform(camLocalPos, localRotMat.Invert()));

            m_Camera->Transform.LocalLookAt(CameraAnchor);
        }
    }    

    Object3DNode::OnUpdate(deltaTime);
}

void ThirdPersonPlayerNode::Destroy(bool keepComponent)
{
	m_Camera = nullptr;
	Object3DNode::Destroy(keepComponent);
}

Node3D* ThirdPersonPlayerNode::Clone(Node3D* newParrent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
    if (!cloneNode)
    {
        cloneNode = new ThirdPersonPlayerNode();
    }

    FirstPersonPlayerNode::Clone(newParrent, cloneChildrenRecursive, cloneNode);

    if (cloneNode)
    {
        ThirdPersonPlayerNode* player = dynamic_cast<ThirdPersonPlayerNode*>(cloneNode);

        player->MinFlyRadius = MinFlyRadius;
        player->MaxFlyRadius = MaxFlyRadius;
        player->CameraAnchor = CameraAnchor;
    }

    return cloneNode;
}

void ThirdPersonPlayerNode::CreateJsonData(json& j)
{
    FirstPersonPlayerNode::CreateJsonData(j);

    j["fly_rad_min"] = MinFlyRadius;
    j["fly_rad_max"] = MaxFlyRadius;
    j["cam_anchor_x"] = CameraAnchor.x;
    j["cam_anchor_y"] = CameraAnchor.y;
    j["cam_anchor_z"] = CameraAnchor.z;
}

void ThirdPersonPlayerNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
    FirstPersonPlayerNode::LoadFromJsonData(nodeData);

    MinFlyRadius = nodeData.MinFlyRadius;
    MaxFlyRadius = nodeData.MaxFlyRadius;
	CameraAnchor = nodeData.CameraAnchor;
}

void ThirdPersonPlayerNode::OnMouseWheel(MouseWheelEventArgs& e)
{
    Object3DNode::OnMouseWheel(e);
    m_FlyRadius = std::clamp(m_FlyRadius - e.WheelDelta * WheelSensitivity, MinFlyRadius, MaxFlyRadius);
}

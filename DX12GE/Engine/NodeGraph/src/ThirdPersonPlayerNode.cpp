#include "../ThirdPersonPlayerNode.h"
#include "../../Base/InputSystem.h"

const static float slowSpeed = 6.0f;
const static float normalSpeed = 12.0f;
const static float fastSpeed = 24.0f;

static inputs is;

ThirdPersonPlayerNode::ThirdPersonPlayerNode() : Object3DNode()
{
	Rename("ThirdPersonPlayerNode");
	m_Direction = Vector3(0.0f, 0.0f, -1.0f);
	m_Speed = normalSpeed;
	m_Camera = nullptr;
}

void ThirdPersonPlayerNode::OnUpdate(const double& deltaTime)
{
    Object3DNode::OnUpdate(deltaTime);

    if (m_test_Enabled)
    {
        TestProcess();
        return;
    }

    ////////////////////////////////
    // ÊÀÌÅÐÀ
    Vector3 playerPos = Transform.GetPosition() + Vector3(0.0f, 2.0f, 0.0f);
    float xTar = cos(m_angle_h) * sin(PI / 2.0f - m_angle_v);
    float yTar = cos(PI / 2.0f - m_angle_v);
    float zTar = sin(m_angle_h) * sin(PI / 2.0f - m_angle_v);
    Vector3 pPosition(playerPos + Vector3(xTar, yTar, zTar) * m_FlyRadius);
    m_Camera->Position = XMVectorSet(pPosition.x, pPosition.y, pPosition.z, 1.0f);

    Vector3 razn = playerPos - pPosition;
    razn.Normalize();
    m_Camera->Target = XMVectorSet(razn.x, razn.y, razn.z, 1.0f);

    ////////////////////////////////////
    //// ÈÃÐÎÊ
    XMVECTOR left = XMVector3Cross(Vector3(m_Camera->Target.m128_f32[0], 0.0f, m_Camera->Target.m128_f32[2]), m_Camera->Up);
    left = XMVector3Normalize(left);

    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

    if (!is.RBC) return;

    m_Speed = is.Shift ? slowSpeed : normalSpeed;
    m_Speed = is.Ctrl ? fastSpeed : normalSpeed;

    if (is.W)
    {
        m_Direction = Vector3(m_Camera->Target.m128_f32[0], 0.0f, m_Camera->Target.m128_f32[2]);
        m_Direction.Normalize();
        Transform.Move(m_Direction * m_Speed * static_cast<float>(deltaTime));
        Transform.SetRotationY(-PI / 2.0f - m_angle_h);
    }
    if (is.S)
    {
        m_Direction = -Vector3(m_Camera->Target.m128_f32[0], 0.0f, m_Camera->Target.m128_f32[2]);
        m_Direction.Normalize();
        Transform.Move(m_Direction * m_Speed * static_cast<float>(deltaTime));
        Transform.SetRotationY(PI / 2.0f - m_angle_h);
    }
    if (is.A)
    {
        m_Direction = left;
        Transform.Move(m_Direction * m_Speed * static_cast<float>(deltaTime));
        Transform.SetRotationY(-PI - m_angle_h);
    }
    if (is.D)
    {
        m_Direction = -left;
        Transform.Move(m_Direction * m_Speed * static_cast<float>(deltaTime));
        Transform.SetRotationY(-m_angle_h);
    }
    if (is.E)
    {
        Transform.Move(m_Camera->Up * m_Speed * static_cast<float>(deltaTime));
    }
    if (is.Q)
    {
        Transform.Move(-m_Camera->Up * m_Speed * static_cast<float>(deltaTime));
    }
}

void ThirdPersonPlayerNode::Destroy(bool keepComponent)
{
	m_Camera = nullptr;
	Object3DNode::Destroy(keepComponent);
}

void ThirdPersonPlayerNode::SetCamera(Camera* camera)
{
    m_Camera = camera;

    if (XMVectorGetZ(camera->Target) >= 0.0f)
    {
        m_angle_h = (XMVectorGetX(camera->Target) >= 0.0f) ? -asin(XMVectorGetZ(camera->Target)) : PI + asin(XMVectorGetZ(camera->Target));
    }
    else
    {
        m_angle_h = (XMVectorGetX(camera->Target) >= 0.0f) ? asin(-XMVectorGetZ(camera->Target)) : PI / 2 + asin(-XMVectorGetZ(camera->Target));
    }
    m_angle_v = asin(XMVectorGetY(camera->Target));
}

void ThirdPersonPlayerNode::OnKeyPressed(KeyEventArgs& e)
{
    Object3DNode::OnKeyPressed(e);

    is.OnKeyPressed(e);
}

void ThirdPersonPlayerNode::OnKeyReleased(KeyEventArgs& e)
{
    Object3DNode::OnKeyReleased(e);

    is.OnKeyReleased(e);
}

void ThirdPersonPlayerNode::OnMouseWheel(MouseWheelEventArgs& e)
{
    Object3DNode::OnMouseWheel(e);

    m_FlyRadius -= e.WheelDelta;
    m_FlyRadius = std::clamp(m_FlyRadius, 5.0f, 40.0f);
}

void ThirdPersonPlayerNode::OnMouseMoved(MouseMotionEventArgs& e)
{
    Object3DNode::OnMouseMoved(e);

    if (!is.RBC) return;

    m_dx = e.X - m_prevX;
    m_dy = e.Y - m_prevY;

    m_angle_h -= m_dx * m_Sensitivity;
    if (m_angle_v + m_dy * m_Sensitivity > -PI / 2.0f && m_angle_v + m_dy * m_Sensitivity < PI / 2.0f)
        m_angle_v += m_dy * m_Sensitivity;

    m_prevX = e.X;
    m_prevY = e.Y;
}

void ThirdPersonPlayerNode::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    Object3DNode::OnMouseButtonPressed(e);

    is.OnMouseButtonPressed(e);

    if (e.Button == 2) //Right mouse
    {
        m_prevX = e.X;
        m_prevY = e.Y;
    }
}

void ThirdPersonPlayerNode::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    Object3DNode::OnMouseButtonReleased(e);

    is.OnMouseButtonReleased(e);
    if (e.Button == 2) //Right mouse
    {
        m_dx = 0;
    }
}

void ThirdPersonPlayerNode::TestProcess()
{
    for (size_t i = 0; i < m_test_MaxPhases; i++)
    {
        if (!m_test_Phases[i].enable) continue;
        m_Camera->Position += m_test_Phases[i].move_target * m_test_MoveStep;

        if (m_Camera->Position.m128_f32[0] < m_test_MapMin.x ||
            m_Camera->Position.m128_f32[0] > m_test_MapMax.x ||
            m_Camera->Position.m128_f32[2] < m_test_MapMin.y ||
            m_Camera->Position.m128_f32[2] > m_test_MapMax.y)
        {
            m_test_Phases[i].enable = false;
            if (i == m_test_MaxPhases - 1)
            {
                m_test_Enabled = false;
                m_Camera->Position = m_test_InitPos;
                m_Camera->Target = m_test_InitTar;
                Transform.SetPosition(m_test_InitPosPlayer);
                Transform.SetRotationY(m_test_InitRotYPlayer);
            }
            else
            {
                m_test_Phases[i + 1].enable = true;
                m_Camera->Position = m_test_Phases[i + 1].start;
                m_Camera->Target = m_test_Phases[i + 1].target;
            }
        }
    }
}

void ThirdPersonPlayerNode::StartTest()
{
    m_test_Enabled = true;

    m_test_InitPos = m_Camera->Position;
    m_test_InitTar = m_Camera->Target;
    m_test_InitPosPlayer = Transform.GetPosition();
    m_test_InitRotYPlayer = Transform.GetRotation().y;
    Transform.SetPosition(m_test_PlayerPosInTest);
    Transform.SetRotation(m_test_PlayerRotInTest);

    m_test_Phases[0].enable = true;
    m_Camera->Position = m_test_Phases[0].start;
    m_Camera->Target = m_test_Phases[0].target;
}

bool ThirdPersonPlayerNode::IsTesting()
{
    return m_test_Enabled;
}

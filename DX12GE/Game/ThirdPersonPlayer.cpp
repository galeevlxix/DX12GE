#include "ThirdPersonPlayer.h"
#include "../Engine/Base/InputSystem.h"

#define PI 3.1415926535f

const static float slowSpeed = 6.0f;
const static float normalSpeed = 12.0f;
const static float fastSpeed = 24.0f;

static inputs is;

void ThirdPersonPlayer::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
	Object3DEntity::OnLoad(commandList, filePath);
	Transform.SetPosition(-6.7f, 13.3f, 43.0f);
	m_Direction = Vector3(0.0f, 0.0f, -1.0f);
	m_Speed = normalSpeed;
}

void ThirdPersonPlayer::OnUpdate(const double& deltaTime)
{
	Object3DEntity::OnUpdate(deltaTime);

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

    m_Speed = is.Shift ? slowSpeed : normalSpeed;
    m_Speed = is.Ctrl ? fastSpeed : normalSpeed;

    if (is.W)
    {
        m_Direction = Vector3(m_Camera->Target.m128_f32[0], 0.0f, m_Camera->Target.m128_f32[2]);
        m_Direction.Normalize();
        Transform.Move(m_Direction * m_Speed * deltaTime);
        Transform.SetRotationY(-PI / 2.0f - m_angle_h);
    }
    if (is.S)
    {
        m_Direction = -Vector3(m_Camera->Target.m128_f32[0], 0.0f, m_Camera->Target.m128_f32[2]);
        m_Direction.Normalize();
        Transform.Move(m_Direction * m_Speed * deltaTime);
        Transform.SetRotationY(PI / 2.0f - m_angle_h);
    }
    if (is.A)
    {
        m_Direction = left;
        Transform.Move(m_Direction * m_Speed * deltaTime);
        Transform.SetRotationY(-PI - m_angle_h);
    }
    if (is.D)
    {
        m_Direction = -left;
        Transform.Move(m_Direction * m_Speed * deltaTime);
        Transform.SetRotationY(-m_angle_h);
    }
    if (is.E)
    {
        Transform.Move(m_Camera->Up * m_Speed * deltaTime);
    }
    if (is.Q)
    {
        Transform.Move(-m_Camera->Up * m_Speed * deltaTime);
    }
}

void ThirdPersonPlayer::SetCamera(Camera* camera)
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

void ThirdPersonPlayer::OnKeyPressed(KeyEventArgs& e)
{
    is.OnKeyPressed(e);
}

void ThirdPersonPlayer::OnKeyReleased(KeyEventArgs& e)
{
    is.OnKeyReleased(e);
}

void ThirdPersonPlayer::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_FlyRadius -= e.WheelDelta;
    m_FlyRadius = clamp(m_FlyRadius, 5.0f, 40.0f);
}

void ThirdPersonPlayer::OnMouseMoved(MouseMotionEventArgs& e)
{
    if (!is.RBC) return;

    m_dx = e.X - m_prevX;
    m_dy = e.Y - m_prevY;

    m_angle_h -= m_dx * m_Sensitivity;
    if (m_angle_v + m_dy * m_Sensitivity > -PI / 2.0f && m_angle_v + m_dy * m_Sensitivity < PI / 2.0f)
        m_angle_v += m_dy * m_Sensitivity;

    m_prevX = e.X;
    m_prevY = e.Y;
}

void ThirdPersonPlayer::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    is.OnMouseButtonPressed(e);
    if (e.Button == 2) //Right mouse
    {
        m_prevX = e.X;
        m_prevY = e.Y;
    }
}

void ThirdPersonPlayer::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    is.OnMouseButtonReleased(e);
    if (e.Button == 2) //Right mouse
    {
        m_dx = 0;
    }
}

void ThirdPersonPlayer::Destroy()
{
    Object3DEntity::Destroy();
    m_Camera = nullptr;
}

//////////////////////////////////
// TESTING MODE

void ThirdPersonPlayer::TestProcess()
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

void ThirdPersonPlayer::StartTest()
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

bool ThirdPersonPlayer::IsTesting()
{
    return m_test_Enabled;
}
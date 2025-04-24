#include "Camera.h"

#define PI 3.1415926535f

// Clamp a value between a min and max range
template<typename T>
constexpr const T& clamp(const T& val, const T& min, const T& max)
{
    return val < min ? min : val > max ? max : val;
}

void Camera::OnLoad(Player* l_player, XMVECTOR position, XMVECTOR target, XMVECTOR up, float fov, float ratio, float zNear, float zFar)
{
    player = l_player;

	Position = position;
	Target = target;
	Target = XMVector3Normalize(Target);
	Up = up;
	Up = XMVector3Normalize(Up);

	Fov = fov;
	Ratio = ratio;
	ZNear = zNear;
	ZFar = zFar;

    if (XMVectorGetZ(Target) >= 0.0f)
    {
        angle_h = (XMVectorGetX(Target) >= 0.0f) ? -asin(XMVectorGetZ(Target)) : PI + asin(XMVectorGetZ(Target));
    }
    else
    {
        angle_h = (XMVectorGetX(Target) >= 0.0f) ? asin(-XMVectorGetZ(Target)) : PI / 2 + asin(-XMVectorGetZ(Target));
    }

    angle_v = asin(XMVectorGetY(Target));
}

XMMATRIX Camera::GetViewProjMatrix()
{
	return XMMatrixMultiply(XMMatrixLookAtLH(Position, Position + Target, Up), XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), Ratio, ZNear, ZFar));
}

XMMATRIX Camera::GetProjMatrix()
{
    return XMMatrixMultiply(XMMatrixLookAtLH(Position, Position + XMVectorSet(0, 0, -1, 0), Up), XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), Ratio, ZNear, ZFar));
}

void Camera::OnUpdate(float deltaTime)
{
    ////////////////////////////////
    // ÊÀÌÅÐÀ
    Vector3 playerPos = (*player).prince.Position + Vector3(0.0f, 2.0f, 0.0f);
    float xTar = cos(angle_h) * sin(PI / 2.0f - angle_v);
    float yTar = cos(PI / 2.0f - angle_v);
    float zTar = sin(angle_h) * sin(PI / 2.0f - angle_v);
    Vector3 pPosition(playerPos + Vector3(xTar, yTar, zTar) * flyRadius);
    Position = XMVectorSet(pPosition.x, pPosition.y, pPosition.z, 1.0f);
    
    Vector3 razn = playerPos - pPosition;
    razn.Normalize();
    Target = XMVectorSet(razn.x, razn.y, razn.z, 1.0f);

    ////////////////////////////////////
    //// ÈÃÐÎÊ
    XMVECTOR left = XMVector3Cross(Vector3(Target.m128_f32[0], 0.0f, Target.m128_f32[2]), Up);
    left = XMVector3Normalize(left);

    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

    if (monitor.W)
    {
        (*player).Direction = Vector3(Target.m128_f32[0], 0.0f, Target.m128_f32[2]);
        (*player).Direction.Normalize();
        (*player).prince.Move((*player).Direction * speed * deltaTime);
        (*player).prince.SetRotationY(-PI / 2.0f - angle_h);
    }
    if (monitor.S) 
    {
        (*player).Direction = -Vector3(Target.m128_f32[0], 0.0f, Target.m128_f32[2]);
        (*player).Direction.Normalize();
        (*player).prince.Move((*player).Direction * speed * deltaTime);
        (*player).prince.SetRotationY(PI / 2.0f - angle_h);
    }
    if (monitor.A)
    {
        (*player).Direction = left;
        (*player).prince.Move((*player).Direction * speed * deltaTime);
        (*player).prince.SetRotationY(-PI - angle_h);
    }
    if (monitor.D)
    {
        (*player).Direction = -left;
        (*player).prince.Move((*player).Direction * speed * deltaTime);
        (*player).prince.SetRotationY(-angle_h);
    }
    if (monitor.E)
    {
        (*player).prince.Move(Up * speed * deltaTime);
    }
    if (monitor.Q)
    {
        (*player).prince.Move(-Up * speed * deltaTime);
    }
}

void Camera::OnMouseWheel(MouseWheelEventArgs& e)
{
    flyRadius -= e.WheelDelta;
    flyRadius = clamp(flyRadius, 5.0f, 40.0f);
}

void Camera::OnMouseMoved(MouseMotionEventArgs& e)
{
    if (!monitor.RBC) return;

    dx = e.X - prevX;
    dy = e.Y - prevY;

    angle_h -= dx * sensitivity;
    if (angle_v + dy * sensitivity > - PI / 2.0f && angle_v + dy * sensitivity < PI / 2.0f)
        angle_v += dy * sensitivity;

    prevX = e.X;
    prevY = e.Y;
}

void Camera::OnKeyPressed(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::W:
        monitor.W = true;
        break;
    case KeyCode::S:
        monitor.S = true;
        break;
    case KeyCode::A:
        monitor.A = true;
        break;
    case KeyCode::D:
        monitor.D = true;
        break;
    case KeyCode::E:
        monitor.E = true;
        break;
    case KeyCode::Q:
        monitor.Q = true;
        break;
    case KeyCode::ShiftKey:
        monitor.Shift = true;
        speed = slowSpeed;
        break;
    case KeyCode::ControlKey:
        monitor.Ctrl = true;
        speed = fastSpeed;
        break;
    }
}

void Camera::OnKeyReleased(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::W:
        monitor.W = false;
        break;
    case KeyCode::S:
        monitor.S = false;
        break;
    case KeyCode::A:
        monitor.A = false;
        break;
    case KeyCode::D:
        monitor.D = false;
        break;
    case KeyCode::E:
        monitor.E = false;
        break;
    case KeyCode::Q:
        monitor.Q = false;
        break;
    case KeyCode::ShiftKey:
        monitor.Shift = false;
        speed = normalSpeed;
        break;
    case KeyCode::ControlKey:
        monitor.Ctrl = false;
        speed = normalSpeed;
        break;
    }
}

void Camera::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    switch (e.Button)
    {
    case 1: //Left
        monitor.LBC = true;
        break;
    case 2: //Right
        monitor.RBC = true;
        prevX = e.X;
        prevY = e.Y;
        break;
    case 3: //Middel
        monitor.MBC = true;
        break;
    }
}

void Camera::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    switch (e.Button)
    {
    case 1: //Left
        monitor.LBC = false;
        break;
    case 2: //Right
        monitor.RBC = false;
        dx = 0;
        break;
    case 3: //Middel
        monitor.MBC = false;
        break;
    }
}

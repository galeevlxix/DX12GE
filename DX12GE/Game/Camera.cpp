#include "Camera.h"

#define PI 3.1415926535f

// Clamp a value between a min and max range
template<typename T>
constexpr const T& clamp(const T& val, const T& min, const T& max)
{
    return val < min ? min : val > max ? max : val;
}

void Camera::OnLoad(XMVECTOR position, XMVECTOR target, XMVECTOR up, float fov, float ratio, float zNear, float zFar, Player* l_player)
{
    player = l_player;

	Position = position;
	Target = target;
	Target = XMVector3Normalize(Target);
	Up = up;
	Up = XMVector3Normalize(Up);

    startFov = fov;
	Fov = fov;
	Ratio = ratio;
	ZNear = zNear;
	ZFar = zFar;

    XMVECTOR HTarget = XMVectorSet(XMVectorGetX(Target), 0, XMVectorGetZ(Target), 1);
    HTarget = XMVector3Normalize(HTarget);

    if (XMVectorGetZ(HTarget) >= 0.0f)
    {
        if (XMVectorGetX(HTarget) >= 0.0f)
        {
            angle_h = 360.0f - XMConvertToDegrees(asin(XMVectorGetZ(HTarget)));
        }
        else
        {
            angle_h = 180.0f + XMConvertToDegrees(asin(XMVectorGetZ(HTarget)));
        }
    }
    else
    {
        if (XMVectorGetX(HTarget) >= 0.0f)
        {
            angle_h = XMConvertToDegrees(asin(-XMVectorGetZ(HTarget)));
        }
        else
        {
            angle_h = 90.0f + XMConvertToDegrees(asin(-XMVectorGetZ(HTarget)));
        }
    }

    angle_v = -XMConvertToDegrees(asin(XMVectorGetY(Target)));
}

XMMATRIX Camera::GetViewProjMatrix()
{
	return XMMatrixMultiply(XMMatrixLookAtLH(Position, Position + Target, Up), XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), Ratio, ZNear, ZFar));
}

void Camera::OnUpdate(float deltaTime)
{
    XMVECTOR left, up;

    Position = Vector3((*player).GetPosition() + Vector3(sin(angle_h) * ((*player).flyRadius + (*player).ballRadius), (*player).GetPosition().Y + 10, cos(angle_h) * ((*player).flyRadius + (*player).ballRadius))).ToXMVector();

    Vector3 razn = (*player).GetPosition() - Vector3(XMVectorGetX(Position), XMVectorGetY(Position), XMVectorGetZ(Position));
    razn.Normalize();
    Target = razn.ToXMVector();

    (*player).Direction = Vector3(Target.m128_f32[0], 0, Target.m128_f32[2]);
    (*player).Angle = angle_h;

    Vector3 forward = (*player).GetPosition() + (*player).Direction * speed * deltaTime;
    bool canGoForward = forward.X < 70 && forward.X > -70 && forward.Z < 70 && forward.Z > -70;
    Vector3 back = (*player).GetPosition() + (*player).Direction * (-speed) * deltaTime;
    bool canGoBack = back.X < 70 && back.X > -70 && back.Z < 70 && back.Z > -70;

    if (monitor.W && canGoForward) (*player).Move((*player).Direction * speed * deltaTime);
    if (monitor.S && canGoBack) (*player).Move((*player).Direction * (-speed) * deltaTime); 

    return;

    if (monitor.W) Position += Target * speed * deltaTime;
    if (monitor.S) Position -= Target * speed * deltaTime;

    if (monitor.A)
    {
        left = XMVector3Cross(Target, Up);
        left = XMVector3Normalize(left);
        Position += left * speed * deltaTime;
    }
    if (monitor.D)
    {
        left = XMVector3Cross(Target, Up);
        left = XMVector3Normalize(left);
        Position -= left * speed * deltaTime;
    }
    if (monitor.E)
    {
        up = XMVectorSet(0, 1, 0, 1);
        Position += up * speed * deltaTime;
    }
    if (monitor.Q)
    {
        up = XMVectorSet(0, 1, 0, 1);
        Position -= up * speed * deltaTime;
    }
}



void Camera::OnMouseWheel(MouseWheelEventArgs& e)
{
    return;
	Fov -= e.WheelDelta;
	Fov = clamp(Fov, 12.0f, 90.0f);

    //std::cout << Fov << std::endl;

    //22
}

void Camera::OnMouseMoved(MouseMotionEventArgs& e)
{
    if (!monitor.RBC) return;

    float dx = e.X - prevX;
    float dy = e.Y - prevY;

    angle_h += dx * sensitivity;
    if (angle_v + dy * sensitivity > -89 && angle_v + dy * sensitivity < 89) 
        angle_v += dy * sensitivity;

    

    
    prevX = e.X;
    prevY = e.Y;

    return;

    float rotX = XMConvertToRadians(angle_h);
    float rotY = XMConvertToRadians(angle_v);

    float x = cos(PI + rotX) * sin(PI / 2 - rotY);
    float y = cos(PI / 2 - rotY);
    float z = sin(PI + rotX) * sin(PI / 2 - rotY);

    Target = XMVectorSet(x, y, z, 1);
    Target = XMVector3Normalize(Target);   
    
    
}

void Camera::OnKeyPressed(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::W:
        monitor.W = true;
        (*player).canRotateForward = true;
        
        break;
    case KeyCode::S:
        monitor.S = true;
        (*player).canRotateBack = true;
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
        speed = slowSpeed * 20;
        break;
    case KeyCode::ControlKey:
        monitor.Ctrl = true;
        speed = fastSpeed * 20;
        break;
    }
}

void Camera::OnKeyReleased(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::W:
        monitor.W = false;
        (*player).canRotateForward = false;
        
        break;
    case KeyCode::S:
        monitor.S = false;
        (*player).canRotateBack = false;
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
        speed = normalSpeed * 20;
        break;
    case KeyCode::ControlKey:
        monitor.Ctrl = false;
        speed = normalSpeed * 20;
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
        break;
    case 3: //Middel
        monitor.MBC = false;
        break;
    }
}

#include "Camera.h"
//#include "../Engine/CommandQueue.h"

#define PI 3.1415926535f

void Camera::OnLoad(XMVECTOR position, XMVECTOR target, XMVECTOR up, float fov, float ratio, float zNear, float zFar)
{
	Position = position;
	Target = target;
	Target = XMVector3Normalize(Target);
	Up = up;
	Up = XMVector3Normalize(Up);

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
    Render();
	return XMMatrixMultiply(XMMatrixLookAtLH(Position, Position + Target, Up), XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), Ratio, ZNear, ZFar));
}

void Camera::Render()
{
    XMVECTOR left, up;
    if (monitor.W) Position += Target * speed;
    if (monitor.S) Position -= Target * speed;
    if (monitor.A)
    {
        left = XMVector3Cross(Target, Up);
        left = XMVector3Normalize(left);
        Position += left * speed;
    }
    if (monitor.D)
    {
        left = XMVector3Cross(Target, Up);
        left = XMVector3Normalize(left);
        Position -= left * speed;
    }
    if (monitor.E)
    {
        up = XMVectorSet(0, 1, 0, 1);
        Position += up * speed;
    }
    if (monitor.Q)
    {
        up = XMVectorSet(0, 1, 0, 1);
        Position -= up * speed;
    }
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

// Clamp a value between a min and max range
template<typename T>
constexpr const T& clamp(const T& val, const T& min, const T& max)
{
	return val < min ? min : val > max ? max : val;
}

void Camera::OnMouseWheel(MouseWheelEventArgs& e)
{
	Fov -= e.WheelDelta;
	Fov = clamp(Fov, 12.0f, 90.0f);
}

void Camera::OnMouseMoved(MouseMotionEventArgs& e)
{
    if (!RBC) return;

    float dx = e.X - prevX;
    float dy = e.Y - prevY;

    angle_h += dx * sensitivity;
    angle_v += dy * sensitivity;

    float rotX = XMConvertToRadians(angle_h);
    float rotY = XMConvertToRadians(angle_v);

    float x = cos(PI + rotX) * sin(PI / 2 - rotY);
    float y = cos(PI / 2 - rotY);
    float z = sin(PI + rotX) * sin(PI / 2 - rotY);

    Target = XMVectorSet(x, y, z, 1);
    Target = XMVector3Normalize(Target);   
    
    prevX = e.X;
    prevY = e.Y;
}

void Camera::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    switch (e.Button)
    {
    case 1: //Left
        
        break;
    case 2: //Right
        RBC = true;
        prevX = e.X;
        prevY = e.Y;
        break;
    case 3: //Middel

        break;
    }
}

void Camera::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    switch (e.Button)
    {
    case 1: //Left

        break;
    case 2: //Right
        RBC = false;
        break;
    case 3: //Middel

        break;
    }
}

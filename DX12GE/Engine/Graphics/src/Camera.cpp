#include "../Camera.h"

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
}

XMMATRIX Camera::GetViewProjMatrix()
{
	return XMMatrixMultiply(XMMatrixLookAtLH(Position, Position + Target, Up), XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), Ratio, ZNear, ZFar));
}

XMMATRIX Camera::GetProjMatrix()
{
    return XMMatrixMultiply(XMMatrixLookAtLH(Position, Position + XMVectorSet(0, 0, -1, 0), Up), XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), Ratio, ZNear, ZFar));
}

void Camera::Destroy()
{
    Position = {};
    Target = {};
    Up = {};

    Fov = {};
    Ratio = {};
    ZNear = {};
    ZFar = {};
}

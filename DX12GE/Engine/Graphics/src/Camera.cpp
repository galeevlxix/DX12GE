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

XMMATRIX Camera::GetViewProjMatrixNoTranslation()
{
	return XMMatrixMultiply(XMMatrixLookAtLH(XMVectorZero(), Target, Up), XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), Ratio, ZNear, 1000));
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

#include "../../Base/Singleton.h"

CameraNode::CameraNode() : Node3D()
{
	m_Type = NODE_TYPE_CAMERA;
	m_Up = Vector3::Up;
	m_Ratio = 1.0f;
	Fov = 60.0f;
	ZNear = 0.1f;
	ZFar = 300.0f;
}

const Matrix CameraNode::GetViewProjMatrix()
{
	Matrix view = Matrix::CreateLookAt(m_WorldPositionCache, m_WorldPositionCache + m_WorldDirectionCache, m_Up);
	Matrix proj = Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(Fov), m_Ratio, ZNear, ZFar);
	return view * proj;
}

const Matrix CameraNode::GetViewProjMatrixNoTranslation()
{
	Matrix viewNoTrans = Matrix::CreateLookAt(Vector3::Zero, m_WorldDirectionCache, m_Up);
	Matrix proj = Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(Fov), m_Ratio, ZNear, 1000.0);
	return viewNoTrans * proj;
}

Node3D* CameraNode::Clone(Node3D* newParrent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
	if (!cloneNode)
	{
		cloneNode = new CameraNode();
	}

	Node3D::Clone(newParrent, cloneChildrenRecursive, cloneNode);

	if (CameraNode* camera = dynamic_cast<CameraNode*>(cloneNode))
	{
		camera->Fov = Fov;
		camera->ZNear = ZNear;
		camera->ZFar = ZFar;
	}

	return cloneNode;
}

void CameraNode::DrawDebug()
{
	Node3D::DrawDebug();
	Matrix view = Matrix::CreateLookAt(m_WorldPositionCache, m_WorldPositionCache + m_WorldDirectionCache, m_Up);
	Matrix proj = Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(Fov), m_Ratio, ZNear, ZFar);
	Singleton::GetDebugRender()->DrawFrustrum(view, proj);
}

void CameraNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);

	j["cam_fov"] = Fov;
	j["cam_z_near"] = ZNear;
	j["cam_z_far"] = ZFar;
}

void CameraNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);
	Fov = nodeData.camFov;
	ZNear = nodeData.camZNear;
	ZFar = nodeData.camZFar;
}

void CameraNode::SetCurrent()
{
	if (FirstPersonPlayerNode* player = dynamic_cast<FirstPersonPlayerNode*>(m_Parrent))
	{
		player->SetCamera(this);
	}
}

bool CameraNode::IsCurrent()
{
	if (FirstPersonPlayerNode* player = dynamic_cast<FirstPersonPlayerNode*>(m_Parrent))
	{
		return player->GetCamera() == this;
	}
	return false;
}

void CameraNode::OnWindowResize(ResizeEventArgs& e)
{
	Node3D::OnWindowResize(e);
	m_Ratio = static_cast<float>(e.Width) / static_cast<float>(e.Height);
}

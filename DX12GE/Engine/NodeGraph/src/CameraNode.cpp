#include "../../Base/Singleton.h"

CameraNode::CameraNode() : Node3D()
{
	m_Type = NODE_TYPE_CAMERA;

	m_Up = Vector3::Up;
	m_Ratio = 1.0f;
	Fov = 60.0f;
	ZNear = 0.1f;
	ZFar = 300.0f;

	Rename("CameraNode");
}

void CameraNode::OnUpdate(const double& deltaTime)
{
	bool dirty = Transform.IsCacheDirty();
	Node3D::OnUpdate(deltaTime);
	if (dirty)
	{
		m_Up = TransformComponent::CalculateUpVector(m_WorldDirectionCache);
		m_ViewMatrixCache = SimpleMath::Matrix::CreateLookAt(m_WorldPositionCache, m_WorldPositionCache + m_WorldDirectionCache, m_Up);
		m_ViewMatrixNoTransCache = SimpleMath::Matrix::CreateLookAt(SimpleMath::Vector3::Zero, m_WorldDirectionCache, m_Up);
	}
}

const SimpleMath::Matrix CameraNode::GetViewProjMatrix()
{
	SimpleMath::Matrix proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(Fov), m_Ratio, ZNear, ZFar);
	return m_ViewMatrixCache * proj;
}

const SimpleMath::Matrix CameraNode::GetViewProjMatrixNoTranslation()
{
	SimpleMath::Matrix proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(Fov), m_Ratio, ZNear, 1000.0);
	return m_ViewMatrixNoTransCache * proj;
}

Node3D* CameraNode::Clone(Node3D* newParent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
	if (!cloneNode)
	{
		cloneNode = new CameraNode();
	}

	Node3D::Clone(newParent, cloneChildrenRecursive, cloneNode);

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
	SimpleMath::Matrix proj = SimpleMath::Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(Fov), m_Ratio, ZNear, ZFar);
	Singleton::GetDebugRender()->DrawFrustrum(m_ViewMatrixCache, proj);
}

void CameraNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);
	j["cam_fov"] = Fov;
	j["cam_z_near"] = ZNear;
	j["cam_z_far"] = ZFar;

	if (IsCurrent())
	{
		j["is_current"] = true;
	}
}

void CameraNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);
	Fov = nodeData.camFov;
	ZNear = nodeData.camZNear;
	ZFar = nodeData.camZFar;

	if (nodeData.isCurrent)
	{
		SetCurrent();
	}
}

void CameraNode::SetCurrent()
{
	if (FirstPersonPlayerNode* player = dynamic_cast<FirstPersonPlayerNode*>(m_Parent))
	{
		player->SetCamera(this);
	}
}

bool CameraNode::IsCurrent()
{
	if (FirstPersonPlayerNode* player = dynamic_cast<FirstPersonPlayerNode*>(m_Parent))
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

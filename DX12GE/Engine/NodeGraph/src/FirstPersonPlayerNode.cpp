#include "../../Base/Singleton.h"

FirstPersonPlayerNode::FirstPersonPlayerNode() : Object3DNode()
{
	m_Type = NODE_TYPE_FIRST_PERSON_PLAYER;
	Rename("FirstPersonPlayerNode");

	m_Camera = nullptr;

	m_CurrentMovementSpeed = 0.0f;
	MinMovementSpeed = 2.0f;
	NormalMovementSpeed = 4.0f;
	MaxMovementSpeed = 8.0f;

	MouseSensitivity = 0.0075f;
	WheelSensitivity = 0.8f;

	m_angle_h = 0.0f;
	m_angle_v = 0.0f;
	m_prevX = 0;
	m_prevY = 0;
	m_dx = 0;
	m_dy = 0;
}

void FirstPersonPlayerNode::OnUpdate(const double& deltaTime)
{
	if (IsCurrent() && m_PressedInputs.RBC)
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

		Transform.SetRotationY(m_angle_h);

		const Matrix& parMat = m_Parrent->GetWorldMatrix();
		Vector3 direction = GetWorldDirection();
		
		if (m_Camera)
		{
			m_Camera->Transform.SetRotationX(m_angle_v);
			direction = m_Camera->GetWorldDirection();
		}

		direction = Vector3::Transform(direction, parMat.Invert());

		Vector3 right = direction.Cross(Vector3::Up);
		right.Normalize();

		float delta = static_cast<float>(deltaTime);
		if (m_PressedInputs.W)
		{
			Transform.Move(direction * m_CurrentMovementSpeed * delta);
		}
		if (m_PressedInputs.S)
		{
			Transform.Move(-direction * m_CurrentMovementSpeed * delta);
		}
		if (m_PressedInputs.A)
		{
			Transform.Move(-right * m_CurrentMovementSpeed * delta);
		}
		if (m_PressedInputs.D)
		{
			Transform.Move(right * m_CurrentMovementSpeed * delta);
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

	Object3DNode::OnUpdate(deltaTime);
}

void FirstPersonPlayerNode::Destroy(bool keepComponent)
{
	m_Camera = nullptr;
	Object3DNode::Destroy(keepComponent);
}

bool FirstPersonPlayerNode::AddChild(Node3D* node)
{
	if (!Object3DNode::AddChild(node)) return false;

	if (!m_Camera)
	{
		if (CameraNode* camera = dynamic_cast<CameraNode*>(node))
		{
			camera->SetRatio(Singleton::GetNodeGraph()->WindowRatio);
			SetCamera(camera);
		}
	}
	return true;
}

Node3D* FirstPersonPlayerNode::Clone(Node3D* newParrent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
	if (!cloneNode)
	{
		cloneNode = new FirstPersonPlayerNode();
	}

	Object3DNode::Clone(newParrent, cloneChildrenRecursive, cloneNode);

	if (cloneNode)
	{
		FirstPersonPlayerNode* player = dynamic_cast<FirstPersonPlayerNode*>(cloneNode);

		player->MouseSensitivity = MouseSensitivity;
		player->WheelSensitivity = WheelSensitivity;
		player->MinMovementSpeed = MinMovementSpeed;
		player->NormalMovementSpeed = NormalMovementSpeed;
		player->MaxMovementSpeed = MaxMovementSpeed;
	}

	return cloneNode;
}

void FirstPersonPlayerNode::CreateJsonData(json& j)
{
	Object3DNode::CreateJsonData(j);

	j["sens_mouse"] = MouseSensitivity;
	j["sens_wheel"] = WheelSensitivity;

	j["speed_min"] = MinMovementSpeed;
	j["speed_nrm"] = NormalMovementSpeed;
	j["speed_max"] = MaxMovementSpeed;

	if (IsCurrent())
	{
		j["is_current"] = true;
	}
}

void FirstPersonPlayerNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Object3DNode::LoadFromJsonData(nodeData);

	MouseSensitivity = nodeData.MouseSensitivity;
	WheelSensitivity = nodeData.WheelSensitivity;
	MinMovementSpeed = nodeData.MinMovementSpeed;
	NormalMovementSpeed = nodeData.NormalMovementSpeed;
	MaxMovementSpeed = nodeData.MaxMovementSpeed;

	if (nodeData.isCurrent)
	{
		SetCurrent();
	}
}

void FirstPersonPlayerNode::SetCurrent()
{
	if (IsInsideTree())
	{
		Singleton::GetNodeGraph()->m_CurrentPlayer = this;
	}
	else
	{
		printf("Внимание! Невозможно сделать FirstPersonPlayerNode::%s активным! Узел не находится в дереве сцены!\n", m_Name.c_str());
	}
}

bool FirstPersonPlayerNode::IsCurrent()
{
	return Singleton::GetNodeGraph()->m_CurrentPlayer == this;
}

void FirstPersonPlayerNode::SetCamera(CameraNode* camera)
{
	if (camera == nullptr || camera->GetParrent() != this) return;
	m_Camera = camera;
	m_angle_h = Transform.GetRotation().y;
	m_angle_v = m_Camera->Transform.GetRotation().x;
}

void FirstPersonPlayerNode::OnKeyPressed(KeyEventArgs& e)
{
	Object3DNode::OnKeyPressed(e);
	m_PressedInputs.OnKeyPressed(e);
}

void FirstPersonPlayerNode::OnKeyReleased(KeyEventArgs& e)
{
	Object3DNode::OnKeyReleased(e);
	m_PressedInputs.OnKeyReleased(e);
}

void FirstPersonPlayerNode::OnMouseMoved(MouseMotionEventArgs& e)
{
	Object3DNode::OnMouseMoved(e);

	if (!m_PressedInputs.RBC) return;

	m_dx = e.X - m_prevX;
	m_dy = e.Y - m_prevY;

	m_angle_h -= m_dx * MouseSensitivity;
	if (m_angle_v + m_dy * MouseSensitivity > -PI / 2.0f && m_angle_v + m_dy * MouseSensitivity < PI / 2.0f)
		m_angle_v += m_dy * MouseSensitivity;

	m_prevX = e.X;
	m_prevY = e.Y;
}

void FirstPersonPlayerNode::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
	Object3DNode::OnMouseButtonPressed(e);
	m_PressedInputs.OnMouseButtonPressed(e);

	if (e.Button == 2) //Right mouse
	{
		m_prevX = e.X;
		m_prevY = e.Y;
	}
}

void FirstPersonPlayerNode::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
	Object3DNode::OnMouseButtonReleased(e);
	m_PressedInputs.OnMouseButtonReleased(e);

	if (e.Button == 2) //Right mouse
	{
		m_dx = 0;
	}
}

void FirstPersonPlayerNode::OnMouseWheel(MouseWheelEventArgs& e)
{
	Object3DNode::OnMouseWheel(e);
	m_Camera->Fov = std::clamp(m_Camera->Fov - e.WheelDelta * WheelSensitivity, 20.0f, 120.0f);
}

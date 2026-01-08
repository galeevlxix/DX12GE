#include "../../Base/Singleton.h"

AudioListenerNode::AudioListenerNode() : Node3D()
{
	m_Type = NODE_TYPE_AUDIO_LISTENER;
	m_PrevWorldPosition = { 0.0f, 0.0f, 0.0f };

	m_ListenerData = X3DAUDIO_LISTENER{};
	m_ListenerData.Position = { 0.0f, 0.0f, 0.0f };
	m_ListenerData.OrientFront = { 0.0f, 0.0f, 1.0f };
	m_ListenerData.OrientTop = { 0.0f, 1.0f, 0.0f };
	m_ListenerData.Velocity = { 0.0f, 0.0f, 0.0f };

	Rename("AudioListenerNode");
}

void AudioListenerNode::OnUpdate(const double& deltaTime)
{
	bool dirty = Transform.IsCacheDirty();
	Node3D::OnUpdate(deltaTime);
	if (dirty)
	{
		m_ListenerData.OrientTop = TransformComponent::CalculateUpVector(m_WorldDirectionCache);
		m_ListenerData.Position = m_WorldPositionCache;
		m_ListenerData.OrientFront = -m_WorldDirectionCache;

		m_ListenerData.Velocity = (m_WorldPositionCache - m_PrevWorldPosition) / static_cast<float>(deltaTime);
		m_PrevWorldPosition = m_WorldPositionCache;
	}
}

void AudioListenerNode::DrawDebug()
{
	Node3D::DrawDebug();
	Singleton::GetDebugRender()->DrawSphere(0.2, Color(1, 1, 0), m_WorldMatrixCache, 8);
}

void AudioListenerNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);

	if (IsCurrent())
	{
		j["is_current"] = true;
	}
}

void AudioListenerNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
	Node3D::LoadFromJsonData(nodeData);

	if (nodeData.isCurrent)
	{
		SetCurrent();
	}
}

void AudioListenerNode::SetCurrent()
{
	if (IsInsideTree())
	{
		Singleton::GetNodeGraph()->m_CurrentListener = this;
	}
	else
	{
		printf("Attention! AudioListenerNode::%s cannot be activated! The node is not located in the scene tree!\n", m_Name.c_str());
	}
}

bool AudioListenerNode::IsCurrent()
{
	return Singleton::GetNodeGraph()->m_CurrentListener == this;
}
#pragma once

#include "Object3DNode.h"

#include "FirstPersonPlayerNode.h"
#include "ThirdPersonPlayerNode.h"
#include "SkyBoxNode.h"

#include "EnvironmentNode.h"
#include "DirectionalLightNode.h"
#include "PointLightNode.h"
#include "SpotLightNode.h"
#include "ParticlesNode.h"
#include "CameraNode.h"
#include "PhysicalObjectNode.h"

#include "../Graphics/GraphicsComponents.h"

#include <vector> 

// ������� ���������� ������� �����
class NodeGraphSystem
{
	Node3D* m_SceneRootNode;

	std::map<std::string, Object3DNode*> m_All3DObjects;
	std::map<std::string, PointLightNode*> m_AllPointLights;
	std::map<std::string, SpotLightNode*> m_AllSpotLights;

	EnvironmentNode* m_CurrentEnvironment;
	DirectionalLightNode* m_CurrentDirectionalLight;
	FirstPersonPlayerNode* m_CurrentPlayer;
	SkyBoxNode* m_CurrentSkyBox;

	EnvironmentNode* m_DefaultEnvironment;
	DirectionalLightNode* m_DefaultDirectionalLight;
	CameraNode* m_DefaultCamera;

	friend void EnvironmentNode::SetCurrent();
	friend bool EnvironmentNode::IsCurrent();

	friend void DirectionalLightNode::SetCurrent();
	friend bool DirectionalLightNode::IsCurrent();
	
	friend void FirstPersonPlayerNode::SetCurrent();
	friend bool FirstPersonPlayerNode::IsCurrent();

	friend void SkyBoxNode::SetCurrent();
	friend bool SkyBoxNode::IsCurrent();

	friend bool Node3D::AddChild(Node3D* node);
	void OnNodeAdded(Node3D* node);

	friend bool Node3D::RemoveChild(const std::string& name);
	void OnNodeRemoved(Node3D* node);

public:
	float WindowRatio = 1.0f;

private:
	const std::vector<Node3D*> GetNodesRecursive(Node3D* current);

public:
	NodeGraphSystem();

	// ���������� �������� ���� �����
	Node3D* GetRoot() { return m_SceneRootNode; }

	// ���������� ������ �����
	void Destroy();
	
	// ���������� ��� ���� ����� � ���� �������
	const std::vector<Node3D*> GetAllNodes() { return GetNodesRecursive(m_SceneRootNode); }

	// ���������� ��� 3� ������� � ����� � ���� ������� (���� � ���� -> ��������� �� ����)
	const std::map<std::string, Object3DNode*>& GetAll3DObjects() { return m_All3DObjects; }

	// ������� ���� �� ��� ���� � ������ �����
	Node3D* GetNodeByPath(const std::string& nodePath);

	// ���������� ��������� ������������� ������ �����
	const std::string Print(Node3D* current = nullptr, int depth = 0);

	// ���������� ������� ���� ��������� �����
	EnvironmentNode* GetCurrentEnvironment();

	// ���������� ������� ���� ������������� ����� �����
	DirectionalLightNode* GetCurrentDirectionalLight();

	// ���������� ������� ���� ������ �����
	FirstPersonPlayerNode* GetCurrentPlayer() { return m_CurrentPlayer; }

	// ���������� ������� ������ �����
	CameraNode* GetCurrentCamera();

	// ���������� ������� ���� ��������� �����
	SkyBoxNode* GetCurrentSkyBox() { return m_CurrentSkyBox; }

	// ���������� ������ ���� ����������� �������� ���������� ����� � �����
	const std::vector<PointLightComponent> GetPointLightComponents();

	// ���������� ������ ���� ����������� ������������ ���������� ����� � �����
	const std::vector<SpotLightComponent> GetSpotLightComponents();

	// ���������� ���������� �������� ���������� ����� � �����
	const size_t GetPointLightsCount() { return m_AllPointLights.size(); }

	// ���������� ���������� ������������ ���������� ����� � �����
	const size_t GetSpotLightsCount() { return m_AllSpotLights.size(); }

	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e) { m_SceneRootNode->OnKeyReleased(e); }
	void OnMouseWheel(MouseWheelEventArgs& e) { m_SceneRootNode->OnMouseWheel(e); }
	void OnMouseMoved(MouseMotionEventArgs& e) { m_SceneRootNode->OnMouseMoved(e); }
	void OnMouseButtonPressed(MouseButtonEventArgs& e) { m_SceneRootNode->OnMouseButtonPressed(e); }
	void OnMouseButtonReleased(MouseButtonEventArgs& e) { m_SceneRootNode->OnMouseButtonReleased(e); }
	void OnResize(ResizeEventArgs& e);

	// ������ ����� ���� � ��������� ��� �� �����
	// ���������� ��������� ����, ���� �������� ������ ������� (nullptr - � ��������� ������)
	Node3D* CreateNewNodeInScene(const std::string& nodePath, NodeTypeEnum type);

	// ������� ���� �� �����
	// ���� destroy = true, ���� ��������� ��������� �� ������
	// ���������� true, ���� �������� ������ �������
	bool RemoveNodeFromScene(const std::string& nodePath, bool destroy = false);

	// ��������� ��������� ���� � ��������� ���� �� �����
	// ���������� ���� �����, ���� �������� ������ ������� (nullptr - � ��������� ������)
	Node3D* CloneNode(const std::string& nodePath, const std::string& pathOfNewParrent);

	// ���������� ���� � ���� ������ ��������
	// ���������� true, ���� ����������� ������ �������
	bool MoveNode(const std::string& nodePath, const std::string& pathOfNewParrent);
};
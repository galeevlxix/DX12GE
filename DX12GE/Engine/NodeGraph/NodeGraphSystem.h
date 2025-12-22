#pragma once

#include "Object3DNode.h"

#include "FirstPersonPlayerNode.h"
#include "ThirdPersonPlayerNode.h"

#include "EnvironmentNode.h"
#include "DirectionalLightNode.h"
#include "PointLightNode.h"
#include "SpotLightNode.h"
#include "ParticlesNode.h"
#include "CameraNode.h"

#include "../Graphics/GraphicsComponents.h"

#include <vector> 

class NodeGraphSystem
{
	Node3D* m_SceneRootNode;

	std::map<std::string, Object3DNode*> m_All3DObjects;
	std::map<std::string, PointLightNode*> m_AllPointLights;
	std::map<std::string, SpotLightNode*> m_AllSpotLights;

	EnvironmentNode* m_CurrentEnvironment;
	friend void EnvironmentNode::SetCurrent();
	friend bool EnvironmentNode::IsCurrent();

	DirectionalLightNode* m_CurrentDirectionalLight;
	friend void DirectionalLightNode::SetCurrent();
	friend bool DirectionalLightNode::IsCurrent();
	
	FirstPersonPlayerNode* m_CurrentPlayer;
	friend void FirstPersonPlayerNode::SetCurrent();
	friend bool FirstPersonPlayerNode::IsCurrent();

	EnvironmentNode* m_DefaultEnvironment;
	DirectionalLightNode* m_DefaultDirectionalLight;
	CameraNode* m_DefaultCamera;

public:
	NodeGraphSystem();

	void SetTree(Node3D* root) { m_SceneRootNode = root; }
	Node3D* GetRoot() { return m_SceneRootNode; }
	void Destroy();

	void OnNodeAdded(Node3D* node);
	void OnNodeRemoved(Node3D* node);

	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e) { m_SceneRootNode->OnKeyReleased(e); }
	void OnMouseWheel(MouseWheelEventArgs& e) { m_SceneRootNode->OnMouseWheel(e); }
	void OnMouseMoved(MouseMotionEventArgs& e) { m_SceneRootNode->OnMouseMoved(e); }
	void OnMouseButtonPressed(MouseButtonEventArgs& e) { m_SceneRootNode->OnMouseButtonPressed(e); }
	void OnMouseButtonReleased(MouseButtonEventArgs& e) { m_SceneRootNode->OnMouseButtonReleased(e); }
	void OnResize(ResizeEventArgs& e) { m_SceneRootNode->OnWindowResize(e); }

	// Возвращает все узлы сцены в виде массива
	const std::vector<Node3D*> GetAllNodes() { return GetNodesRecursive(m_SceneRootNode); }

	// Возвращает все 3Д объекты в сцене в виде словаря (путь к узлу -> указатель на узел)
	const std::map<std::string, Object3DNode*>& GetAll3DObjects() { return m_All3DObjects; }

	Node3D* GetNodeByPath(const std::string& nodePath);

	const std::string Print(Node3D* current = nullptr, int depth = 0);

private:
	
	const std::vector<Node3D*> GetNodesRecursive(Node3D* current);

public:

	float WindowRatio = 1.0f;

	EnvironmentNode* GetCurrentEnvironment();
	DirectionalLightNode* GetCurrentDirectionalLight();
	FirstPersonPlayerNode* GetCurrentPlayer() { return m_CurrentPlayer; }
	CameraNode* GetCurrentCamera();

	const std::vector<PointLightComponent> GetPointLightComponents();
	const std::vector<SpotLightComponent> GetSpotLightComponents();

	const size_t GetPointLightsCount() { return m_AllPointLights.size(); }
	const size_t GetSpotLightsCount() { return m_AllSpotLights.size(); }
};
#pragma once

#include "Object3DNode.h"
#include "ThirdPersonPlayerNode.h"
#include "ParticlesNode.h"
#include "EnvironmentNode.h"
#include "DirectionalLightNode.h"
#include "PointLightNode.h"
#include "SpotLightNode.h"
#include "CameraNode.h"

#include <vector> 

class NodeGraphSystem
{
	Node3D* m_SceneRootNode;
	std::map<std::string, Object3DNode*> m_All3DObjects;

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

	const std::vector<Node3D*> GetAllNodes();
	const std::map<std::string, Object3DNode*>& GetAll3DObjects();

	Node3D* GetNodeByPath(const std::string& nodePath);

	const std::string Print(Node3D* current = nullptr, int depth = 0);

private:
	
	const std::vector<Node3D*> GetNodesRecursive(Node3D* current);
};
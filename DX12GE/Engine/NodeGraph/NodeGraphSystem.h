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
#include "AudioListenerNode.h"
#include "AudioEmitterNode.h"
#include "PhysicalObjectNode.h"

#include "../Graphics/GraphicsComponents.h"

#include <vector> 

/// \brief Scene tree management system
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

	std::map<std::string, AudioEmitterNode*> m_AllAudioEmitters;
	AudioListenerNode* m_CurrentListener;

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

	friend void AudioListenerNode::SetCurrent();
	friend bool AudioListenerNode::IsCurrent();

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

	/// \brief Returns the root node of the scene.
	Node3D* GetRoot() { return m_SceneRootNode; }

	/// \brief Reset the scene tree.
	void Reset(bool keepComponent = true);

	/// \brief Destroys the scene tree.
	void Destroy();
	
	/// \brief Returns all scene nodes as a list.
	const std::vector<Node3D*> GetAllNodes() { return GetNodesRecursive(m_SceneRootNode); }

	/// \brief Returns all 3D objects in the scene as a dictionary (path to node -> node).
	const std::map<std::string, Object3DNode*>& GetAll3DObjects() { return m_All3DObjects; }

	/// \brief Finds a node along its path in the scene tree.
	/// \return Returns the node if it is found. Returns nullptr otherwise.
	Node3D* GetNodeByPath(const std::string& nodePath);

	/// \brief Returns a string representation of the scene tree.
	const std::string Print(Node3D* current = nullptr, int depth = 0);

	/// \brief Returns the current scene environment node.
	/// \return Returns the active environment if it exists. Returns default environment otherwise.
	EnvironmentNode* GetCurrentEnvironment();

	/// \brief Returns the current directional light node of the scene.
	/// \return Returns the active directional light if it exists. Returns default directional light otherwise.
	DirectionalLightNode* GetCurrentDirectionalLight();

	/// \brief Returns the current player node of the scene.
	/// \return Returns the active player if it exists. Returns nullptr otherwise.
	FirstPersonPlayerNode* GetCurrentPlayer() { return m_CurrentPlayer; }

	/// \brief Returns the current camera node of the scene.
	/// \return Returns the active camera if it exists. Returns default camera otherwise.
	CameraNode* GetCurrentCamera();

	/// \brief Returns the current skybox node of the scene.
	/// \return Returns the active skybox if it exists. Returns nullptr otherwise.
	SkyBoxNode* GetCurrentSkyBox() { return m_CurrentSkyBox; }

	/// \brief Returns an array of all point light source components in the scene.
	const std::vector<PointLightComponent> GetPointLightComponents();

	/// \brief Returns an array of all spotlight light source components in the scene.
	const std::vector<SpotLightComponent> GetSpotLightComponents();

	/// \brief Returns the number of point light sources in the scene.
	const size_t GetPointLightsCount() { return m_AllPointLights.size(); }

	/// \brief Returns the number of spotlight light sources in the scene.
	const size_t GetSpotLightsCount() { return m_AllSpotLights.size(); }

	/// \brief Returns all sound sources (audio emitters) in the scene as a dictionary (path to node -> node).
	const std::map<std::string, AudioEmitterNode*>& GetAllAudioEmitters() { return m_AllAudioEmitters; }

	/// \brief Returns the current listener node of the scene.
	/// \return Returns the active listener if it exists. Returns nullptr otherwise.
	AudioListenerNode* GetCurrentListener() { return m_CurrentListener; }

	void OnKeyPressed(KeyEventArgs& e);
	void OnKeyReleased(KeyEventArgs& e) { m_SceneRootNode->OnKeyReleased(e); }
	void OnMouseWheel(MouseWheelEventArgs& e) { m_SceneRootNode->OnMouseWheel(e); }
	void OnMouseMoved(MouseMotionEventArgs& e) { m_SceneRootNode->OnMouseMoved(e); }
	void OnMouseButtonPressed(MouseButtonEventArgs& e) { m_SceneRootNode->OnMouseButtonPressed(e); }
	void OnMouseButtonReleased(MouseButtonEventArgs& e) { m_SceneRootNode->OnMouseButtonReleased(e); }
	void OnResize(ResizeEventArgs& e);

	/// \brief Creates a new node and adds it to the scene.
	/// \param nodePath Sets the parent and name of the new node.
	/// \param type Node type for the new node.
	/// \return Returns the created node if creation was successful. Returns nullptr otherwise.
	Node3D* CreateNewNodeInScene(const std::string& nodePath, NodeTypeEnum type);

	/// \brief Removes the node from the scene.
	/// \param nodePath Node path of the node.
	/// \param destroy If it is true, the node's component data is completely removed from memory.
	/// \return Returns true if the deletion was successful. Returns false otherwise.
	bool RemoveNodeFromScene(const std::string& nodePath, bool destroy = false);

	/// \brief Completely clones the node and adds the clone to the scene.
	/// \param nodePath Node path of the original node.
	/// \param pathOfNewParent Node path of the new parent for this clone.
	/// \return Returns the clone node if creation was successful. Returns nullptr otherwise.
	Node3D* CloneNode(const std::string& nodePath, const std::string& pathOfNewParent);

	/// \brief Moves the node to the new parent node
	/// \param nodePath Node path of the node.
	/// \param pathOfNewParent Node path of the new parent for this node.
	/// \return Returns true if the move was successful. Returns false otherwise.
	bool MoveNode(const std::string& nodePath, const std::string& pathOfNewParent);
};
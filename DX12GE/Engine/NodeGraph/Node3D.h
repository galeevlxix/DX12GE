#pragma once

#include "../Graphics/TransformComponent.h"
#include "NodeSerializingData.h"
#include "../Base/Events.h"
#include <map>
#include <vector>
#include <stdexcept>

/// \brief Base class of a node in the scene tree.
class Node3D
{
public:
	/// \brief Modifying the transformation of the node (position, rotation, scale).
	/// \note Transform changes will only take effect after OnUpdate is called.
	TransformComponent Transform;

protected:
	/// \brief Direct descendants of the node.
	/// \warning Direct modification of the this parameter is not permitted. Use the AddChild, RemoveChild, Move, or Clone methods instead.
	/// \see AddChild, RemoveChild, Move, Clone
	std::map<std::string, Node3D*> m_Children;

	/// \brief Direct parent of the node.
	/// \warning Direct modification of the this parameter is not permitted. Use the AddChild, RemoveChild, Move, or Clone methods instead.
	/// \see AddChild, RemoveChild, Move, Clone
	Node3D* m_Parent = nullptr;

	/// \brief Name of the node.
	/// \warning Direct modification of the this parameter is not permitted. Use the Rename method instead.
	/// \see Rename
	std::string m_Name;

	/// \brief Transform matrix in world space.
	/// \warning Direct modification of the this parameter is not permitted. This parameter is automatically updated after calling OnUpdate.
	/// \see OnUpdate
	DirectX::XMMATRIX m_WorldMatrixCache;

	/// \brief World position vector.
	/// \warning Direct modification of the this parameter is not permitted. This parameter is automatically updated after calling OnUpdate.
	/// \see OnUpdate
	DirectX::SimpleMath::Vector3 m_WorldPositionCache;

	/// \brief World direction vector.
	/// \warning Direct modification of the this parameter is not permitted. This parameter is automatically updated after calling OnUpdate.
	/// \see OnUpdate
	DirectX::SimpleMath::Vector3 m_WorldDirectionCache;

	/// \brief Type of the node.
	/// \warning Modification of the this parameter is not permitted.
	NodeTypeEnum m_Type;

public:
	Node3D();

	/// \brief Returns the node type.
	NodeTypeEnum GetType() { return m_Type; }

	/// \brief Returns the path of a node in the scene tree from the farthest parent to the given node. For example, "root/road/car/driver".
	const std::string GetNodePath();

	/// \brief Returns the Lua scripts attached to this node as a single string.
	const std::string GetNodeScripts();
	
	/// \brief Called every tick on a node update.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	virtual void OnUpdate(const double& deltaTime);

	/// \brief Destroys this node and all descendants of the node. The node is also removed from its parent's descendant list.
	/// \param keepComponent When it is false, the component with data is also removed from memory if there are no other nodes in the scene tree that use this component (keepComponent applies to descendants)
	virtual void Destroy(bool keepComponent = true);
    
	/// \brief Returns the world matrix of the node.
	/// \note Only updated after OnUpdate.
	/// \see OnUpdate
	const DirectX::XMMATRIX& GetWorldMatrix();
	
	/// \brief Returns the position of an node in world space.
	/// \note Only updated after OnUpdate.
	/// \see OnUpdate
	const DirectX::SimpleMath::Vector3& GetWorldPosition() { return m_WorldPositionCache; }

	/// \brief Returns the direction of an node in world space (Vector3(0, 0, 1) -> world space).
	/// \note Only updated after OnUpdate.
	/// \see OnUpdate
	const DirectX::SimpleMath::Vector3& GetWorldDirection() { return m_WorldDirectionCache; }

	/// \brief Returns the name of the node.
	const std::string GetName() { return m_Name; }
	
	/// \brief Renames this node.
	/// \param name New name of this node
	/// \note If there is already a descendant with this name among the direct descendants of this node's parent, a number is added to the new name to avoid name repetition among the parent's descendants.
	/// \throws std::invalid_argument If name contain the characters: '/', '%'.
	void Rename(const std::string& name);

	/// \brief Checks whether the node is in the tree.
	/// \return Returns true if the node is in the scene tree. Returns false otherwise.
	bool IsInsideTree();

	/// \brief Returns a direct descendant with given name.
	/// \param name Name of descendant
	/// \return Returns nullptr if this node does not contain a descendant with given name. 
	Node3D* GetChild(const std::string& name);

	/// \brief Returns all descendants of this node.
	/// \return Returns an empty list if the node has no descendants.
	const std::vector<Node3D*> GetChildren();

	/// \brief Returns the names of all descendants of this node.
	/// \return Returns an empty list if the node has no descendants.
	const std::vector<std::string> GetChildrenNames();

	/// \brief Returns the direct parent.
	/// \return Returns nullptr if this node does not have Parent.
	Node3D* GetParent();

	/// \brief Finds the first node whose name matches name.
	/// \param name Name of descendant node
	/// \return Returns nullptr if a node with that name is not found.
	/// \note This method may be slow.
	Node3D* FindNodeRecursive(const std::string& name);

	/// Finds the first parent whose name matches the given name.
	/// \param name Name of Parent
	Node3D* FindParentRecursive(const std::string& name);

	/// \brief Adds a new direct descendant to this node.
	/// \param node New descendant node
	/// \return Returns true if the addition was successful. Returns false otherwise.
	/// \note The descendant will not be added if it already has a parent. Use the Move method in this case.
	virtual bool AddChild(Node3D* node);

	/// \brief Removes a node from the list of direct descendants by name, but does not destroy the descendant itself.
	/// \return Returns true if the removal was successful. Returns false otherwise.
	bool RemoveChild(const std::string& name);

	/// \brief Removes a node from the list of direct descendants, but does not destroy the descendant itself.
	/// \param node Descendant node
	/// \return Returns true if the removal was successful. Returns false otherwise.
	bool RemoveChild(Node3D* node);

	/// \brief Looking for a direct descendant.
	/// \param name Name of descendant node
	/// \return Returns true if a direct descendant with that name is found. Returns false otherwise.
	bool HasChild(const std::string& name);

	/// \brief Moves this node to the new parent node.
	/// \param newParent Parent node
	/// \return Returns true if moving was successful. Returns false otherwise.
	bool Move(Node3D* newParent);
	
	/// \brief Clones this node by copying data and parameters.
	/// \code
	/// // Example of use:  
	/// Node3D* newNode = oldNode->Clone(someParent, true);
	/// \endcode
	/// \param newParent If it is defined, the new node will be added to this parent node.
	/// \param cloneChildrenRecursive If it is true, all descendants are also cloned.
	/// \param cloneNode should be nullptr
	/// \return Returns a new node if cloning was successful. Returns nullptr otherwise.
	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr);

	/// \brief Recursively draws debugging primitives.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	/// \warning DEBUG MODE ONLY.
	virtual void DrawDebug();

	/// \brief Makes the node the only active one on the scene.
	/// \see CameraNode, DirectionalLightNode, EnvironmentNode, SkyBoxNode, AudioListenerNode, FirstPersonPlayerNode, ThirdPersonPlayerNode
	virtual void SetCurrent() {};

	/// \brief Creates JSON data for serializing a node.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	virtual void CreateJsonData(json& j);

	/// \brief Loads node data from the NodeSerializingData structure.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	virtual void LoadFromJsonData(const NodeSerializingData& nodeData);

private:
	void NotifyParentChanged();
	void RenameChild(const std::string& oldName, const std::string& newName);
	
public:

	/// \brief Called when the keyboard key is pressed.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	/// \warning DEBUG MODE ONLY.
	virtual void OnKeyPressed(KeyEventArgs& e);

	/// \brief Called when the keyboard key is released.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	/// \warning DEBUG MODE ONLY.
	virtual void OnKeyReleased(KeyEventArgs& e);

	/// \brief Called when the mouse wheel is moved.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	/// \warning DEBUG MODE ONLY.
	virtual void OnMouseWheel(MouseWheelEventArgs& e);

	/// \brief Called when the mouse is moved.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	/// \warning DEBUG MODE ONLY.
	virtual void OnMouseMoved(MouseMotionEventArgs& e);
	
	/// \brief Called when the mouse button is pressed.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	/// \warning DEBUG MODE ONLY.
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
	
	/// \brief Called when the mouse button is released.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	/// \warning DEBUG MODE ONLY.
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
	
	/// \brief Called when the window size changes.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	/// \warning DEBUG MODE ONLY.
	virtual void OnWindowResize(ResizeEventArgs& e);

private:
	uint32_t m_UniqueID;

public:
	/// \brief Returns unique ID of the node.
	uint32_t GetNodeId() { return m_UniqueID; }
};
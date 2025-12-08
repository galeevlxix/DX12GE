#include "../NodeGraphSystem.h"
#include "../../Base/Singleton.h"

NodeGraphSystem::NodeGraphSystem()
{
	m_SceneRootNode = new Node3D();
	m_SceneRootNode->Rename("root");
}

void NodeGraphSystem::Destroy()
{
	m_SceneRootNode->Destroy(false);
	delete m_SceneRootNode;
	m_SceneRootNode = nullptr;	

	m_All3DObjects.clear();
}

void NodeGraphSystem::OnNodeAdded(Node3D* node)
{
	if (node->IsInsideTree())
	{
		Object3DNode* obj3D = dynamic_cast<Object3DNode*>(node);
		if (obj3D)
		{
			m_All3DObjects[obj3D->GetNodePath()] = obj3D;
		}

		for (Node3D* child : node->GetChildren())
		{
			OnNodeAdded(child);
		}
	}
}

void NodeGraphSystem::OnNodeRemoved(Node3D* node)
{
	if (node->IsInsideTree())
	{
		auto obj = m_All3DObjects.find(node->GetNodePath());
		if (obj != m_All3DObjects.end())
		{
			m_All3DObjects.erase(obj->first);
		}

		for (Node3D* child : node->GetChildren())
		{
			OnNodeRemoved(child);
		}
	}
}

const std::vector<Node3D*> NodeGraphSystem::GetAllNodes()
{
	return GetNodesRecursive(m_SceneRootNode);
}

const std::map<std::string, Object3DNode*>& NodeGraphSystem::GetAll3DObjects()
{
	return m_All3DObjects;
}

Node3D* NodeGraphSystem::GetNodeByPath(const std::string& nodePath)
{
	std::vector<std::string> arr;
	size_t prev = 0;
	size_t next;
	while ((next = nodePath.find('/', prev)) != std::string::npos)
	{
		arr.push_back(nodePath.substr(prev, next - prev));
		prev = next + 1;
	}
	arr.push_back(nodePath.substr(prev));

	if (arr[0] != "root") return nullptr;
	Node3D* current = m_SceneRootNode;
	for (int i = 1; i < arr.size(); i++)
	{
		current = current->GetChild(arr[i]);
	}
	return current;
}

const std::string NodeGraphSystem::Print(Node3D* current, int depth)
{
	current = current == nullptr ? GetRoot() : current;

	std::string str = "";
	for (int i = 0; i < depth; i++)
	{
		str += "\t";
	}
	str = str + current->GetName() + "\n";

	for (Node3D* child : current->GetChildren())
	{
		str += Print(child, depth + 1);
	}

	return str;
}

const std::vector<Node3D*> NodeGraphSystem::GetNodesRecursive(Node3D* current)
{
	std::vector<Node3D*> output;
	output.push_back(current);	
	for (Node3D* child : current->GetChildren())
	{
		for (Node3D* n : GetNodesRecursive(child))
		{
			output.push_back(n);
		}
	}
	return output;
}

void NodeGraphSystem::OnKeyPressed(KeyEventArgs& e) 
{ 
	m_SceneRootNode->OnKeyPressed(e); 

	if (e.Key == KeyCode::Delete)
	{
		auto objects = Singleton::GetSelection()->GetSelected();
		for (int i = 0; i < objects.size(); i++)
		{
			objects[i]->Destroy(false);
		}
		Singleton::GetSelection()->DeselectAll();
	}
}
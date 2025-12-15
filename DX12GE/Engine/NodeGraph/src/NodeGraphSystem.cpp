#include "../NodeGraphSystem.h"
#include "../../Base/Singleton.h"

NodeGraphSystem::NodeGraphSystem()
{
	m_SceneRootNode = new Node3D();
	m_SceneRootNode->Rename("root");

	m_ActiveEnvironment = nullptr;
	m_ActiveDirectionalLight = nullptr;
}

void NodeGraphSystem::Destroy()
{
	m_SceneRootNode->Destroy(false);
	delete m_SceneRootNode;
	m_SceneRootNode = nullptr;	

	m_All3DObjects.clear();
	m_AllPointLights.clear();
	m_AllSpotLights.clear();

	m_ActiveEnvironment = nullptr;
	m_ActiveDirectionalLight = nullptr;
}

bool NodeGraphSystem::OnNodeAdded(Node3D* node)
{
	if (node->IsInsideTree())
	{
		const std::string nodePath = node->GetNodePath();

		if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(node))
		{
			m_All3DObjects[nodePath] = obj3D;

		}
		else if (EnvironmentNode* env = dynamic_cast<EnvironmentNode*>(node))
		{
			if (!m_ActiveEnvironment)
			{
				m_ActiveEnvironment = env;
			}
			else
			{
				return false;
			}
		}
		else if (DirectionalLightNode* dirLight = dynamic_cast<DirectionalLightNode*>(node))
		{
			if (!m_ActiveDirectionalLight)
			{
				m_ActiveDirectionalLight = dirLight;
			}
			else
			{
				return false;
			}
		}
		else if (PointLightNode* pLight = dynamic_cast<PointLightNode*>(node))
		{
			m_AllPointLights[nodePath] = pLight;
		}
		else if (SpotLightNode* sLight = dynamic_cast<SpotLightNode*>(node))
		{
			m_AllSpotLights[nodePath] = sLight;
		}			

		for (Node3D* child : node->GetChildren())
		{
			bool result = OnNodeAdded(child);
			if (!result) 
				return false;
		}
	}

	return true;
}

void NodeGraphSystem::OnNodeRemoved(Node3D* node)
{
	if (node->IsInsideTree())
	{
		const std::string nodePath = node->GetNodePath();

		if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(node))
		{
			if (m_All3DObjects.contains(nodePath))
			{
				m_All3DObjects.erase(nodePath);
			}
		}
		else if (EnvironmentNode* env = dynamic_cast<EnvironmentNode*>(node))
		{
			if (env == m_ActiveEnvironment)
			{
				m_ActiveEnvironment = nullptr;
			}
		}
		else if (DirectionalLightNode* dirLight = dynamic_cast<DirectionalLightNode*>(node))
		{
			if (dirLight == m_ActiveDirectionalLight)
			{
				m_ActiveDirectionalLight = nullptr;
			}
		}
		else if (PointLightNode* pLight = dynamic_cast<PointLightNode*>(node))
		{
			if (m_AllPointLights.contains(nodePath))
			{
				m_AllPointLights.erase(nodePath);
			}
		}
		else if (SpotLightNode* sLight = dynamic_cast<SpotLightNode*>(node))
		{
			if (m_AllSpotLights.contains(nodePath))
			{
				m_AllSpotLights.erase(nodePath);
			}
		}			

		for (Node3D* child : node->GetChildren())
		{
			OnNodeRemoved(child);
		}
	}
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

void NodeGraphSystem::SetActiveEnvironmentExplicitly(EnvironmentNode* env)
{
	if (env->IsInsideTree())
	{
		m_ActiveEnvironment = env;
	}
}

void NodeGraphSystem::SetActiveDirectionalLightExplicitly(DirectionalLightNode* dirLight)
{
	if (dirLight->IsInsideTree())
	{
		m_ActiveDirectionalLight = dirLight;
	}
}

const std::vector<PointLightComponent> NodeGraphSystem::GetActivePointLightComponents()
{
	vector<PointLightComponent> result;
	for (auto pLight : m_AllPointLights)
	{
		result.push_back(pLight.second->LightData);
	}
	return result;
}

const std::vector<SpotLightComponent> NodeGraphSystem::GetActiveSpotLightComponents()
{
	vector<SpotLightComponent> result;
	for (auto sLight : m_AllSpotLights)
	{
		result.push_back(sLight.second->LightData);
	}
	return result;
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
#include "../NodeGraphSystem.h"
#include "../../Base/Singleton.h"

NodeGraphSystem::NodeGraphSystem()
{
	m_SceneRootNode = new Node3D();
	m_SceneRootNode->Rename("root");

	m_CurrentEnvironment = nullptr;
	m_CurrentDirectionalLight = nullptr;
	m_CurrentPlayer = nullptr;

	m_DefaultEnvironment = nullptr;
	m_DefaultDirectionalLight = nullptr;
	m_DefaultCamera = nullptr;
}

void NodeGraphSystem::Destroy()
{
	m_SceneRootNode->Destroy(false);
	delete m_SceneRootNode;
	m_SceneRootNode = nullptr;	

	m_All3DObjects.clear();
	m_AllPointLights.clear();
	m_AllSpotLights.clear();

	m_CurrentEnvironment = nullptr;
	m_CurrentDirectionalLight = nullptr;
	m_CurrentPlayer = nullptr;

	if (m_DefaultEnvironment)
	{
		delete m_DefaultEnvironment;
		m_DefaultEnvironment = nullptr;
	}	

	if (m_DefaultDirectionalLight)
	{
		delete m_DefaultDirectionalLight;
		m_DefaultDirectionalLight = nullptr;
	}
	
	if (m_DefaultCamera)
	{
		delete m_DefaultCamera;
		m_DefaultCamera = nullptr;
	}	
}

void NodeGraphSystem::OnNodeAdded(Node3D* node)
{
	if (node->IsInsideTree())
	{
		const std::string nodePath = node->GetNodePath();

		if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(node))
		{
			m_All3DObjects[nodePath] = obj3D;
		}
		else if (PointLightNode* pLight = dynamic_cast<PointLightNode*>(node))
		{
			m_AllPointLights[nodePath] = pLight;
		}
		else if (SpotLightNode* sLight = dynamic_cast<SpotLightNode*>(node))
		{
			m_AllSpotLights[nodePath] = sLight;
		}
		else if (!m_CurrentEnvironment)
		{
			if (EnvironmentNode* env = dynamic_cast<EnvironmentNode*>(node)) 
				m_CurrentEnvironment = env;
		}
		else if (!m_CurrentDirectionalLight)
		{
			if (DirectionalLightNode* dirLight = dynamic_cast<DirectionalLightNode*>(node))
			{
				m_CurrentDirectionalLight = dirLight;
			}
		}

		if (!m_CurrentPlayer)
		{
			if (FirstPersonPlayerNode* player = dynamic_cast<FirstPersonPlayerNode*>(node))
			{
				m_CurrentPlayer = player;
			}
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
		const std::string nodePath = node->GetNodePath();

		if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(node))
		{
			if (m_All3DObjects.contains(nodePath))
			{
				m_All3DObjects.erase(nodePath);
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
		else if (node == m_CurrentEnvironment)
		{
			m_CurrentEnvironment = nullptr;
		}
		else if (node == m_CurrentDirectionalLight)
		{
			m_CurrentDirectionalLight = nullptr;
		}
		else if (node == m_CurrentPlayer)
		{
			m_CurrentPlayer = nullptr;
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

EnvironmentNode* NodeGraphSystem::GetCurrentEnvironment()
{
	if (m_CurrentEnvironment)
	{
		return m_CurrentEnvironment;
	}
	else
	{
		if (!m_DefaultEnvironment)
		{
			m_DefaultEnvironment = new EnvironmentNode();
			m_DefaultEnvironment->OnUpdate(0.0f);
		}
		return m_DefaultEnvironment;
	}
}

DirectionalLightNode* NodeGraphSystem::GetCurrentDirectionalLight()
{
	if (m_CurrentDirectionalLight)
	{
		return m_CurrentDirectionalLight;
	}
	else
	{
		if (!m_DefaultDirectionalLight)
		{
			m_DefaultDirectionalLight = new DirectionalLightNode();
			m_DefaultDirectionalLight->OnUpdate(0.0f);
		}
		return m_DefaultDirectionalLight;
	}
}

CameraNode* NodeGraphSystem::GetCurrentCamera()
{
	if (m_CurrentPlayer)
	{
		if (CameraNode* camera = m_CurrentPlayer->GetCamera())
		{
			return camera;
		}
	}

	if (!m_DefaultCamera)
	{
		m_DefaultCamera = new CameraNode();
		m_DefaultCamera->OnUpdate(0.0f);
	}
	return m_DefaultCamera;
}

const std::vector<PointLightComponent> NodeGraphSystem::GetPointLightComponents()
{
	vector<PointLightComponent> result;
	for (auto pLight : m_AllPointLights)
	{
		result.push_back(pLight.second->LightData);
	}
	return result;
}

const std::vector<SpotLightComponent> NodeGraphSystem::GetSpotLightComponents()
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
			if (objects[i])
				objects[i]->Destroy(false);
		}
		Singleton::GetSelection()->DeselectAll();
	}
}
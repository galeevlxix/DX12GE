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

		if (SkyBoxNode* skybox = dynamic_cast<SkyBoxNode*>(node))
		{
			if (!m_CurrentSkyBox)
			{
				m_CurrentSkyBox = skybox;
			}
		}
		else if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(node))
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
		else if (node == m_CurrentSkyBox)
		{
			m_CurrentSkyBox = nullptr;
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
		if (!current) return nullptr;
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

	std::vector<Node3D*> objects;

	switch (e.Key)
	{
	case KeyCode::Delete:
		objects = Singleton::GetSelection()->GetSelected();
		Singleton::GetSelection()->DeselectAll();
		for (int i = 0; i < objects.size(); i++)
		{
			if (objects[i])
				objects[i]->Destroy(false);
		}
		break;

	case KeyCode::C:
		if (e.Alt)
		{
			objects = Singleton::GetSelection()->GetSelected();
			Singleton::GetSelection()->DeselectAll();
			for (int i = 0; i < objects.size(); i++)
			{
				if (objects[i])
				{
					Node3D* clone = objects[i]->Clone(nullptr, true);
					if (clone)
					{
						Singleton::GetSelection()->SelectedNode(clone);
					}
				}
			}
		}
		break;
	}
}

void NodeGraphSystem::OnResize(ResizeEventArgs& e) 
{ 
	m_SceneRootNode->OnWindowResize(e); 

	WindowRatio = static_cast<float>(e.Width) / static_cast<float>(e.Height); 
}

Node3D* NodeGraphSystem::CreateNewNodeInScene(const std::string& nodePath, NodeTypeEnum type)
{
	Node3D* node = nullptr;

	if (GetNodeByPath(nodePath))
	{
		printf("Ошибка! Узел %s уже существует!\n", nodePath.c_str());
		return node;
	}

	ParsedNodePath parsed;
	parsed.ParseNodePath(nodePath);

	if (parsed.name == "" || parsed.parrentNodePath == "")
	{
		printf("Ошибка! Невозможно создать узел %s!\n", nodePath.c_str());
		return node;
	}	

	switch (type)
	{
	case NODE_TYPE_NODE3D:
		node = new Node3D();
		break;
	case NODE_TYPE_OBJECT3D:
		node = new Object3DNode();
		break;
	case NODE_TYPE_FIRST_PERSON_PLAYER:
		node = new FirstPersonPlayerNode();
		break;
	case NODE_TYPE_THIRD_PERSON_PLAYER:
		node = new ThirdPersonPlayerNode();
		break;
	case NODE_TYPE_ENVIRONMENT:
		node = new EnvironmentNode();
		break;
	case NODE_TYPE_DIRECTIONAL_LIGHT:
		node = new DirectionalLightNode();
		break;
	case NODE_TYPE_POINT_LIGHT:
		node = new PointLightNode();
		break;
	case NODE_TYPE_SPOT_LIGHT:
		node = new SpotLightNode();
		break;
	case NODE_TYPE_CAMERA:
		node = new CameraNode();
		break;
	case NODE_TYPE_SKYBOX:
		node = new SkyBoxNode();
		break;
	default:
		printf("Ошибка! Тип узла %d не поддерживается!\n", type);
		return node;
	}

	Node3D* parrent = GetNodeByPath(parsed.parrentNodePath);

	if (!parrent)
	{	
		if (node)
		{
			delete node;
			node = nullptr;
		}
		printf("Ошибка! Родительский узел %s не существует!\n", parsed.parrentNodePath.c_str());
		return node;
	}

	node->Rename(parsed.name);
	
	if (!parrent->AddChild(node))
	{
		delete node;
		node = nullptr;

		printf("Ошибка! Невозможно добавить узел %s в родительский узел %s!\n", parsed.name.c_str(), parsed.parrentNodePath.c_str());
	}

	return node;
}

bool NodeGraphSystem::RemoveNodeFromScene(const std::string& nodePath, bool destroy)
{
	Node3D* node = GetNodeByPath(nodePath);

	if (!node)
	{
		printf("Ошибка! Узла %s не существует в сцене!\n", nodePath.c_str());
		return false;
	}

	if (destroy)
	{
		node->Destroy(false);
		return true;
	}
	
	Node3D* parrent = node->GetParrent();
	return node->RemoveChild(node);
}

Node3D* NodeGraphSystem::CloneNode(const std::string& nodePath, const std::string& pathOfNewParrent)
{
	Node3D* original = GetNodeByPath(nodePath);
	if (!original)
	{
		printf("Ошибка! Узла %s не существует в сцене!\n", nodePath.c_str());
		return nullptr;
	}

	Node3D* newParrent = GetNodeByPath(pathOfNewParrent);
	if (!newParrent)
	{
		printf("Ошибка! Узла %s не существует в сцене!\n", pathOfNewParrent.c_str());
		return nullptr;
	}

	Node3D* clone = original->Clone(newParrent, true);

	if (!clone)
	{
		printf("Ошибка! Не удалось создать клон!\n");
		return nullptr;
	}

	return clone;
}

bool NodeGraphSystem::MoveNode(const std::string& nodePath, const std::string& pathOfNewParrent)
{
	Node3D* node = GetNodeByPath(nodePath);
	if (!node)
	{
		printf("Ошибка! Узла %s не существует в сцене!\n", nodePath.c_str());
		return false;
	}

	Node3D* newParrent = GetNodeByPath(pathOfNewParrent);
	if (!newParrent)
	{
		printf("Ошибка! Узла %s не существует в сцене!\n", pathOfNewParrent.c_str());
		return false;
	}

	return node->Move(newParrent);
}

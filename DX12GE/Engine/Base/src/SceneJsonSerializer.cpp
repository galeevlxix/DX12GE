#include "../SceneJsonSerializer.h"
#include "../../Graphics/ResourceStorage.h"
#include "../Singleton.h"
#include <string>
#include "../json.hpp"
#include <fstream>

using json = nlohmann::json;
using Vector3 = DirectX::SimpleMath::Vector3;

static const std::string path = "../../DX12GE/Resources/scene lite.json";

struct ParsedNodePath
{
	std::string name;
	std::string parrentNodePath;
};

struct NodeData
{
	std::string nodePath;
	std::string type;
	std::string filePath;

	Vector3 pos;
	Vector3 rot;
	Vector3 scl;
};

static const ParsedNodePath ParseNodePath(const std::string& nodePath)
{
	ParsedNodePath output;

	const size_t last_slash_idx = nodePath.rfind('/');
	if (last_slash_idx != string::npos)
	{
		output.name = nodePath.substr(last_slash_idx + 1);
		output.parrentNodePath = nodePath.substr(0, last_slash_idx);
	}
	else
	{
		output.name = nodePath;
		output.parrentNodePath = "";
	}

	return output;
}

void SceneJsonSerializer::Save()
{
	std::ofstream out;
	out.open(path);
	json scene;

	for (auto obj : Singleton::GetNodeGraph()->GetAllNodes())
	{
		json entity;

		entity["node_path"] = obj->GetNodePath();
		entity["type"] = obj->GetType();

		if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(obj))
		{
			entity["file_path"] = obj3D->GetObjectFilePath();
		}
		else
		{
			entity["file_path"] = "";
		}	

		auto pos = obj->Transform.GetPosition();
		auto rot = obj->Transform.GetRotation();
		auto scl = obj->Transform.GetScale();

		entity["posX"] = pos.x;
		entity["posY"] = pos.y;
		entity["posZ"] = pos.z;
		entity["rotX"] = rot.x;
		entity["rotY"] = rot.y;
		entity["rotZ"] = rot.z;
		entity["sclX"] = scl.x;
		entity["sclY"] = scl.y;
		entity["sclZ"] = scl.z;

		scene.push_back(entity);
	}
	
	out << scene.dump(4);
	out.close();
}

void SceneJsonSerializer::Load(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	std::ifstream in;
	in.open(path);

	json scene;
	in >> scene;

	std::cout << "Начало загрузки объектов сцены из файла " + path << std::endl;

	std::vector<NodeData> nodesData;

	for (json::iterator it = scene.begin(); it != scene.end(); ++it)
	{
		NodeData newNode;
		newNode.nodePath = it->at("node_path");
		newNode.type = it->at("type");
		newNode.filePath = it->at("file_path");
		newNode.pos = Vector3(it->at("posX"), it->at("posY"), it->at("posZ"));
		newNode.rot = Vector3(it->at("rotX"), it->at("rotY"), it->at("rotZ"));
		newNode.scl = Vector3(it->at("sclX"), it->at("sclY"), it->at("sclZ"));
		nodesData.push_back(newNode);
	}

	for (int i = 0; i < nodesData.size() - 1; ++i)
	{
		for (int j = 0; j < nodesData.size() - i - 1; ++j)
		{
			if (std::count(nodesData[j].nodePath.begin(), nodesData[j].nodePath.end(), '/') >
				std::count(nodesData[j + 1].nodePath.begin(), nodesData[j + 1].nodePath.end(), '/'))
			{
				std::swap(nodesData[j], nodesData[j + 1]);
			}
		}
	}

	std::map<std::string, Node3D*> createdNodes;

	if (nodesData[0].type == "Node3D" && nodesData[0].nodePath == "root")
	{
		createdNodes["root"] = Singleton::GetNodeGraph()->GetRoot();
	}
	else
	{
		throw "Ошибка! Файл поврежден! Файл сцены не содержит коренвой узел";
	}	

	for (int i = 1; i < nodesData.size(); ++i)
	{
		NodeData nodeData = nodesData[i];
		ParsedNodePath parsed = ParseNodePath(nodeData.nodePath);

		Node3D* node;

		if (nodeData.type == "Node3D")
		{
			node = new Node3D();
			node->OnLoad();
		}
		else if (nodeData.type == "Object3DNode" || nodeData.type == "ThirdPersonPlayerNode")
		{
			Object3DNode* obj3D = nodeData.type == "ThirdPersonPlayerNode" ? new ThirdPersonPlayerNode() : new Object3DNode();
			std::string modelPath = nodeData.filePath;
			obj3D->Create(commandList, modelPath);
			node = obj3D;
		}
		else
		{
			throw "Ошибка! Данный тип узла не поддерживается";
		}

		node->Transform.SetPosition(nodeData.pos);
		node->Transform.SetRotation(nodeData.rot);
		node->Transform.SetScale(nodeData.scl);

		auto parrent = createdNodes.find(parsed.parrentNodePath);
		if (parrent != createdNodes.end())
		{
			node->Rename(parsed.name);
			parrent->second->AddChild(node);
			createdNodes[nodeData.nodePath] = node;
		}
		else
		{
			throw "Ошибка! Файл сцены поврежден!";
		}
	}

	std::cout << "Конец загрузки объектов сцены." << std::endl;

	in.close();
}

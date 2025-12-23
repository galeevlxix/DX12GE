#include "../SceneJsonSerializer.h"
#include "../../Graphics/ResourceStorage.h"
#include "../Singleton.h"
#include <fstream>

static const std::string path = "../../DX12GE/Resources/scene lite.json";

struct ParsedNodePath
{
	std::string name;
	std::string parrentNodePath;
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
		if (obj == nullptr) continue;
		
		json entity;
		obj->CreateJsonData(entity);
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

	std::vector<NodeSerializingData> nodesData;

	for (json::iterator it = scene.begin(); it != scene.end(); ++it)
	{
		NodeSerializingData newNode;

		newNode.nodePath = it->at("node_path");
		newNode.type = it->at("node_type");

		newNode.filePath = it->contains("file_path") ? it->at("file_path") : "";

		newNode.pos = Vector3(it->at("trans_pos_x"), it->at("trans_pos_y"), it->at("trans_pos_z"));
		newNode.rot = Vector3(it->at("trans_rot_x"), it->at("trans_rot_y"), it->at("trans_rot_z"));
		newNode.scl = Vector3(it->at("trans_scl_x"), it->at("trans_scl_y"), it->at("trans_scl_z"));

		if (it->contains("is_current"))
		{
			newNode.isCurrent = true;
		}

		if (it->contains("is_visible"))
		{
			newNode.isVisible = it->at("is_visible");
		}

		// lights
		if (it->contains("light_color_r") && it->contains("light_color_g") && it->contains("light_color_b"))
		{
			newNode.lightColor = Vector3(it->at("light_color_r"), it->at("light_color_g"), it->at("light_color_b"));
		}
		
		if (it->contains("light_intensity"))
		{
			newNode.lightIntensity = it->at("light_intensity");
		}
		
		if (it->contains("light_atten_constant") && it->contains("light_atten_linear") && it->contains("light_atten_exp"))
		{
			newNode.lightAttenuation = Vector3(it->at("light_atten_constant"), it->at("light_atten_linear"), it->at("light_atten_exp"));
		}

		if (it->contains("light_cutoff"))
		{
			newNode.lightCutoff = it->at("light_cutoff");
		}
		
		//environment
		if (it->contains("fog_enabled"))
		{
			newNode.envFogEnabled = it->at("fog_enabled");
		}

		if (it->contains("fog_start"))
		{
			newNode.envFogStart = it->at("fog_start");
		}

		if (it->contains("fog_distance"))
		{
			newNode.envFogDistance = it->at("fog_distance");
		}

		if (it->contains("fog_color_r") && it->contains("fog_color_g") && it->contains("fog_color_b"))
		{
			newNode.envFogColor = Vector3(it->at("fog_color_r"), it->at("fog_color_g"), it->at("fog_color_b"));
		}

		if (it->contains("ssr_max_distance"))
		{
			newNode.envSSRMaxDistance = it->at("ssr_max_distance");
		}

		if (it->contains("ssr_step_length"))
		{
			newNode.envSSRStepLength = it->at("ssr_step_length");
		}

		if (it->contains("ssr_thickness"))
		{
			newNode.envSSRThickness = it->at("ssr_thickness");
		}

		// camera

		if (it->contains("cam_fov"))
		{
			newNode.camFov = it->at("cam_fov");
		}

		if (it->contains("cam_z_near"))
		{
			newNode.camZNear = it->at("cam_z_near");
		}

		if (it->contains("cam_z_far"))
		{
			newNode.camZFar = it->at("cam_z_far");
		}

		// player
		if (it->contains("sens_mouse"))
		{
			newNode.MouseSensitivity = it->at("sens_mouse");
		}

		if (it->contains("sens_wheel"))
		{
			newNode.WheelSensitivity = it->at("sens_wheel");
		}

		if (it->contains("speed_min"))
		{
			newNode.MinMovementSpeed = it->at("speed_min");
		}

		if (it->contains("speed_nrm"))
		{
			newNode.NormalMovementSpeed = it->at("speed_nrm");
		}

		if (it->contains("speed_max"))
		{
			newNode.MaxMovementSpeed = it->at("speed_max");
		}

		if (it->contains("fly_rad_min"))
		{
			newNode.MinFlyRadius = it->at("fly_rad_min");
		}

		if (it->contains("fly_rad_max"))
		{
			newNode.MaxFlyRadius = it->at("fly_rad_max");
		}

		if (it->contains("cam_anchor_x") && it->contains("cam_anchor_y") && it->contains("cam_anchor_z"))
		{
			newNode.CameraAnchor = Vector3(it->at("cam_anchor_x"), it->at("cam_anchor_y"), it->at("cam_anchor_z"));
		}

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

	if (nodesData[0].type == NODE_TYPE_NODE3D && nodesData[0].nodePath == "root")
	{
		createdNodes["root"] = Singleton::GetNodeGraph()->GetRoot();
	}
	else
	{
		throw "Ошибка! Файл поврежден! Файл сцены не содержит коренвой узел";
	}

	for (int i = 1; i < nodesData.size(); ++i)
	{
		NodeSerializingData nodeData = nodesData[i];
		ParsedNodePath parsed = ParseNodePath(nodeData.nodePath);
		
		Node3D* node = nullptr;

		switch(nodeData.type)
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
		default:
			printf("Ошибка! Тип узла %d не поддерживается!\n", nodeData.type);
			break;
		}

		if (!node) continue;

		if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(node))
		{
			if (!obj3D->Create(commandList, nodeData.filePath))
			{
				printf("Предупреждение! Меш узла %s не инициализирован!\n", parsed.name.c_str());
			}
		}

		node->LoadFromJsonData(nodeData);

		auto parrent = createdNodes.find(parsed.parrentNodePath);
		if (parrent != createdNodes.end())
		{
			node->Rename(parsed.name);
			parrent->second->AddChild(node);
			createdNodes[nodeData.nodePath] = node;

			if (nodeData.isCurrent)
			{
				node->SetCurrent();
			}
		}
		else
		{
			printf("Ошибка! Файл сцены поврежден!\n");
			node->Destroy(true);
			delete node;
		}
	}

	std::cout << "Конец загрузки объектов сцены." << std::endl;

	in.close();
}

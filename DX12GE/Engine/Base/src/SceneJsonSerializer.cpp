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

	// lights
	Vector3 lightColor;
	float lightIntensity;
	Vector3 lightAttenuation;
	float lightCutoff;

	//environment
	bool envFogEnabled;
	Vector3 envFogColor;
	float envFogStart;
	float envFogDistance;

	float envSSRMaxDistance;
	float envSSRStepLength;
	float envSSRThickness;
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

		entity["node_path"] = obj->GetNodePath();
		entity["type"] = obj->GetType();

		if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(obj))
		{
			entity["file_path"] = obj3D->GetObjectFilePath();
		}
		else if (EnvironmentNode* env = dynamic_cast<EnvironmentNode*>(obj))
		{
			auto color = env->AmbientLightColor;
			entity["light_color_r"] = color.x;
			entity["light_color_g"] = color.y;
			entity["light_color_b"] = color.z;
			entity["light_intensity"] = env->AmbientLightIntensity;

			entity["fog_enabled"] = env->FogEnabled;
			auto fogColor = env->FogColor;
			entity["fog_color_r"] = fogColor.x;
			entity["fog_color_g"] = fogColor.y;
			entity["fog_color_b"] = fogColor.z;
			entity["fog_start"] = env->FogStart;
			entity["fog_distance"] = env->FogDistance;

			entity["ssr_max_distance"] = env->SSRMaxDistance;
			entity["ssr_step_length"] = env->SSRStepLength;
			entity["ssr_thickness"] = env->SSRThickness;
		}
		else if (DirectionalLightNode* dLight = dynamic_cast<DirectionalLightNode*>(obj))
		{
			auto color = dLight->Color;
			entity["light_color_r"] = color.x;
			entity["light_color_g"] = color.y;
			entity["light_color_b"] = color.z;
			entity["light_intensity"] = dLight->Intensity;
		}
		else if (PointLightNode* pLight = dynamic_cast<PointLightNode*>(obj))
		{
			auto color = pLight->LightData.BaseLightProperties.Color;
			entity["light_color_r"] = color.x;
			entity["light_color_g"] = color.y;
			entity["light_color_b"] = color.z;
			entity["light_intensity"] = pLight->LightData.BaseLightProperties.Intensity;

			auto atten = pLight->LightData.AttenuationProperties;
			entity["light_atten_constant"] = atten.Constant;
			entity["light_atten_linear"] = atten.Linear;
			entity["light_atten_exp"] = atten.Exp;
		}
		else if (SpotLightNode* sLight = dynamic_cast<SpotLightNode*>(obj))
		{
			auto color = sLight->LightData.PointLightProperties.BaseLightProperties.Color;
			entity["light_color_r"] = color.x;
			entity["light_color_g"] = color.y;
			entity["light_color_b"] = color.z;
			entity["light_intensity"] = sLight->LightData.PointLightProperties.BaseLightProperties.Intensity;

			auto atten = sLight->LightData.PointLightProperties.AttenuationProperties;
			entity["light_atten_constant"] = atten.Constant;
			entity["light_atten_linear"] = atten.Linear;
			entity["light_atten_exp"] = atten.Exp;

			entity["light_cutoff"] = sLight->LightData.Cutoff;
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

		newNode.filePath = it->contains("file_path") ? it->at("file_path") : "";

		newNode.pos = Vector3(it->at("posX"), it->at("posY"), it->at("posZ"));
		newNode.rot = Vector3(it->at("rotX"), it->at("rotY"), it->at("rotZ"));
		newNode.scl = Vector3(it->at("sclX"), it->at("sclY"), it->at("sclZ"));

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
		else if (nodeData.type == "EnvironmentNode")
		{
			EnvironmentNode* env = new EnvironmentNode();
			env->AmbientLightColor = nodeData.lightColor;
			env->AmbientLightIntensity = nodeData.lightIntensity;
			env->FogColor = nodeData.envFogColor;
			env->FogEnabled = nodeData.envFogEnabled;
			env->FogStart = nodeData.envFogStart;
			env->FogDistance = nodeData.envFogDistance;

			env->SSRMaxDistance = nodeData.envSSRMaxDistance;
			env->SSRStepLength = nodeData.envSSRStepLength;
			env->SSRThickness = nodeData.envSSRThickness;

			node = env;
		}
		else if (nodeData.type == "DirectionalLightNode")
		{
			DirectionalLightNode* dLight = new DirectionalLightNode();
			dLight->Color = nodeData.lightColor;
			dLight->Intensity = nodeData.lightIntensity;
			node = dLight;
		}
		else if (nodeData.type == "PointLightNode")
		{
			PointLightNode* pLight = new PointLightNode();
			pLight->LightData.BaseLightProperties.Color = nodeData.lightColor;
			pLight->LightData.BaseLightProperties.Intensity = nodeData.lightIntensity;
			pLight->LightData.AttenuationProperties.Constant = nodeData.lightAttenuation.x;
			pLight->LightData.AttenuationProperties.Linear = nodeData.lightAttenuation.y;
			pLight->LightData.AttenuationProperties.Exp = nodeData.lightAttenuation.z;
			node = pLight;
		}
		else if (nodeData.type == "SpotLightNode")
		{
			SpotLightNode* sLight = new SpotLightNode();
			sLight->LightData.PointLightProperties.BaseLightProperties.Color = nodeData.lightColor;
			sLight->LightData.PointLightProperties.BaseLightProperties.Intensity = nodeData.lightIntensity;
			sLight->LightData.PointLightProperties.AttenuationProperties.Constant = nodeData.lightAttenuation.x;
			sLight->LightData.PointLightProperties.AttenuationProperties.Linear = nodeData.lightAttenuation.y;
			sLight->LightData.PointLightProperties.AttenuationProperties.Exp = nodeData.lightAttenuation.z;
			sLight->LightData.Cutoff = nodeData.lightCutoff;
			node = sLight;
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

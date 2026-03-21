#include "../SceneJsonSerializer.h"
#include "../../Graphics/ResourceStorage.h"
#include "../../Base/LuaManager.h"
#include "../Singleton.h"
#include "../DX12GE/EngineConfig.h"
#include <fstream>

struct NodeData
{
	std::string nodePath;
	std::string type;
	std::vector<std::string> scripts;
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
		output.ParentNodePath = nodePath.substr(0, last_slash_idx);
	}
	else
	{
		output.name = nodePath;
		output.ParentNodePath = "";
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

	if (!in.is_open())
	{
		std::cout << "Error! Could not open scene file " + path << std::endl;
		return;
	}

	json scene;
	in >> scene;

	std::cout << "Start loading scene objects from file " + path << std::endl;

	std::vector<NodeSerializingData> nodesData;

	for (json::iterator it = scene.begin(); it != scene.end(); ++it)
	{
		NodeSerializingData newNode;

		newNode.nodePath = it->at("node_path");
		
		newNode.type = it->at("node_type");

		if (it->contains("scripts"))
		{
			newNode.scripts = it->at("scripts");
		}	
		
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

		if (it->contains("audio_volume"))
		{
			newNode.audioVolume = it->at("audio_volume");
		}

		if (it->contains("audio_pitch"))
		{
			newNode.audioPitch = it->at("audio_pitch");
		}

		if (it->contains("audio_loop"))
		{
			newNode.audioLoop = it->at("audio_loop");
		}

		if (it->contains("audio_doppler"))
		{
			newNode.audioDoppler = it->at("audio_doppler");
		}

		if (it->contains("audio_ubiquitous"))
		{
			newNode.audioUbiquitous = it->at("audio_ubiquitous");
		}
		
		//Physics
		if (it->contains("collision_type"))
		{
			newNode.collisionType = it->at("collision_type");
		}
		
		if (it->contains("DOF"))
		{
			newNode.DOF = it->at("DOF");
		}
				
		if (it->contains("gravity_scale"))
		{
			newNode.gravityScale = it->at("gravity_scale");
		}
		
		if (it->contains("mass"))
		{
			newNode.mass = it->at("mass");
		}
		
		if (it->contains("friction_scale"))
		{
			newNode.frictionScale = it->at("friction_scale");
		}

		// materials 

		if (it->contains("materials_override") && it->at("materials_override").is_array())
		{
			for (const auto& mat : it->at("materials_override"))
			{
				MaterialSerializingData material;

				if (mat.contains("index"))
				{
					material.index = mat.at("index");
				}
				else
				{
					continue;
				}

				if (mat.contains("name"))
				{
					material.Name = mat.at("name");
				}

				if (mat.contains("diffuse_map"))
				{
					material.Diffuse = mat.at("diffuse_map");
				}

				if (mat.contains("emissive_map"))
				{
					material.Emissive = mat.at("emissive_map");
				}

				if (mat.contains("normal_map"))
				{
					material.Normal = mat.at("normal_map");
				}

				if (mat.contains("metallic_map"))
				{
					material.Metallic = mat.at("metallic_map");
				}

				if (mat.contains("roughness_map"))
				{
					material.Roughness = mat.at("roughness_map");
				}

				if (mat.contains("metallic_roughness_map"))
				{
					material.GltfMetallicRoughness = mat.at("metallic_roughness_map");
				}

				if (mat.contains("ao_map"))
				{
					material.AmbientOcclusion = mat.at("ao_map");
				}

				if (mat.contains("albedo_color_x") && mat.contains("albedo_color_y") && mat.contains("albedo_color_z"))
				{
					material.AlbedoColor = Vector3(mat.at("albedo_color_x"), mat.at("albedo_color_y"), mat.at("albedo_color_z"));
				}

				newNode.materials_override.push_back(material);
			}
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

	if (nodesData[0].type != NODE_TYPE_NODE3D || nodesData[0].nodePath != "root")
	{
		throw "Error! File is corrupted! Scene file does not contain a root node!";
	}

	for (int i = 1; i < nodesData.size(); ++i)
	{
		NodeSerializingData nodeData = nodesData[i];
		
		Node3D* node = Singleton::GetNodeGraph()->CreateNewNodeInScene(nodeData.nodePath, nodeData.type);

		if (!node) continue;

		if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(node))
		{
			if (!obj3D->Create(commandList, nodeData.filePath))
			{
				printf("Warning! The mesh node %s has not been initialized!\n", node->GetName().c_str());
			}
			else
			{
				obj3D->LoadOverrideMaterials(commandList, nodeData);
			}
		}
		else if (AudioEmitterNode* emit = dynamic_cast<AudioEmitterNode*>(node))
		{
			emit->LoadWav(nodeData.filePath);
			emit->SpawnPlayingSound(false);
		}

		node->LoadFromJsonData(nodeData);
	}

	std::cout << "End of scene object loading." << std::endl;

	if (!EngineConfig::IsReleaseMode)
	{
		for (const auto& node : nodesData)
		{
			for (const auto& script : node.scripts)
			{
				LuaManager::CreateValidClass(script, node.nodePath, node.type);
			}
		}
	}

	in.close();
}

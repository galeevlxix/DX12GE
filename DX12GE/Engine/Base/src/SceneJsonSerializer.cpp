#include "../SceneJsonSerializer.h"

#include "../../../Game/ThirdPersonPlayer.h"
#include "../../Graphics/ResourceStorage.h"

#include "../json.hpp"
#include <fstream>
#include <iostream>
#include <chrono>

using json = nlohmann::json;

static const std::string path = "../../DX12GE/Resources/scene.json";

void SceneJsonSerializer::Save(std::map<std::string, Object3DEntity*>& objects)
{
	std::ofstream out;
	out.open(path);
	json scene;

	for (auto obj : objects)
	{
		json entity;	
		
		entity["name"] = obj.first;
		entity["path"] = ResourceStorage::GetObject3D(obj.second->GetId())->ResourcePath;

		auto pos = obj.second->Transform.GetPosition();
		auto rot = obj.second->Transform.GetRotation();
		auto scl = obj.second->Transform.GetScale();

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

void SceneJsonSerializer::Load(ComPtr<ID3D12GraphicsCommandList2> commandList, std::map<std::string, Object3DEntity*>& objects)
{
	std::ifstream in;
	in.open(path);

	json scene;
	in >> scene;

	std::string prevPath = "";
	std::cout << "Начало загрузки объектов сцены из файла " + path << std::endl;
	auto start = std::chrono::steady_clock::now();
	
	for (json::iterator it = scene.begin(); it != scene.end(); ++it)
	{
		std::string name = it->at("name");
		if (objects.find(name) != objects.end()) { continue; }

		Object3DEntity* obj = name == "player" ? new ThirdPersonPlayer() : new Object3DEntity();

		objects.insert({ name, obj });
		std::string modelPath = it->at("path");

		if (modelPath != prevPath)
		{
			float progress = (float)(it - scene.begin()) / (float)(scene.end() - scene.begin());
			std::cout << (int)(progress * 100) << "%: Загрузка компонента " + modelPath << std::endl;

			prevPath = modelPath;
		}

		objects[name]->OnLoad(commandList, modelPath);

		objects[name]->Transform.SetPosition(DirectX::SimpleMath::Vector3(it->at("posX"), it->at("posY"), it->at("posZ")));
		objects[name]->Transform.SetRotation(DirectX::SimpleMath::Vector3(it->at("rotX"), it->at("rotY"), it->at("rotZ")));
		objects[name]->Transform.SetScale(DirectX::SimpleMath::Vector3(it->at("sclX"), it->at("sclY"), it->at("sclZ")));
	}

	auto end = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
	std::cout << "Конец загрузки объектов сцены. Затраченное время: " << duration << "s" << std::endl;

	in.close();
}

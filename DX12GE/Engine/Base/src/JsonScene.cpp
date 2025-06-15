#include "../JsonScene.h"
#include "../json.hpp"
#include "../../Graphics/ResourceStorage.h"
#include <fstream>

using json = nlohmann::json;

void JsonScene::Save(std::map<std::string, Object3DEntity>& objects)
{
	std::ofstream out;
	out.open(path);
	json scene;

	for (auto obj : objects)
	{
		json entity;	
		entity["name"] = obj.first;
		entity["path"] = ResourceStorage::GetObject3D(obj.second.GetId())->ResourcePath;
		entity["posX"] = obj.second.Transform.GetPosition().x;
		entity["posY"] = obj.second.Transform.GetPosition().y;
		entity["posZ"] = obj.second.Transform.GetPosition().z;
		entity["rotX"] = obj.second.Transform.GetRotation().x;
		entity["rotY"] = obj.second.Transform.GetRotation().y;
		entity["rotZ"] = obj.second.Transform.GetRotation().z;
		entity["sclX"] = obj.second.Transform.GetScale().x;
		entity["sclY"] = obj.second.Transform.GetScale().y;
		entity["sclZ"] = obj.second.Transform.GetScale().z;
		scene.push_back(entity);
	}
	
	out << scene.dump(4);
	out.close();
}

void JsonScene::Load(ComPtr<ID3D12GraphicsCommandList2> commandList, std::map<std::string, Object3DEntity>& objects)
{
	std::ifstream in;
	in.open(path);

	json scene;
	in >> scene;
	
	for (json::iterator it = scene.begin(); it != scene.end(); ++it)
	{
		std::string name = it->at("name");
		if (objects.find(name) != objects.end()) { continue; }

		objects.insert({ name, Object3DEntity() });
		std::string path = it->at("path");
		objects[name].OnLoad(commandList, path);

		auto pos = DirectX::SimpleMath::Vector3(it->at("posX"), it->at("posY"), it->at("posZ"));
		auto rot = DirectX::SimpleMath::Vector3(it->at("rotX"), it->at("rotY"), it->at("rotZ"));
		auto scl = DirectX::SimpleMath::Vector3(it->at("sclX"), it->at("sclY"), it->at("sclZ"));
		objects[name].Transform.SetPosition(pos);
		objects[name].Transform.SetRotation(rot);
		objects[name].Transform.SetScale(scl);
	}

	in.close();
}

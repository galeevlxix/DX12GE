#include <iostream>
#include <typeinfo>
#include <string>
#include "../SingleGpuGame.h"
#include <vector>
#include <cctype>
#include "../DX12GE/Engine/NodeGraph/NodeTypeEnum.h"
#include <filesystem>
#include "../DX12GE/EngineConfig.h"
#include <algorithm>
#include <system_error>
#include <direct.h> 
#include "../DX12GE/Engine/NodeGraph/AINode.h"
#include "LuaManager.h"
//#define SOL_ALL_SAFETIES_ON 0

namespace fs = std::filesystem;

#define TEST

lua_State* LuaManager::L = nullptr;
LuaManager* LuaManager::p_instance = nullptr;
std::string const luaSciptsFolder = "../GameEngineDev/x64/Debug/scripts/";
static SingleGpuGame* p_scene;
static NodeGraphSystem* p_grapsh_system;
static std::vector<std::string> lua_classes_vector;
static std::vector<std::string> lua_file_classes;
static std::map<std::string, std::vector<std::string>> lua_classes_map;
static std::map<std::string, std::vector<std::string>> node_path_to_classes;
static sol::state lua;
//////////////////////////////////////////////////////////////////////
//LUA API ZONE
Node3D* lua_get_object_on_scene(std::string name)
{
	const auto& object = p_grapsh_system->GetNodeByPath(name);

	return object;
}

int lua_get_node_type(Node3D* object)
{
	assert(object != nullptr, "Attempt to call get node type on null object!");

	return object->GetType();
}

int lua_destroy_node_by_node(Node3D* object)
{
	//	assert(object != nullptr, "Attempt to call destroy on null object!");

	const std::string path{ object->GetNodePath() };
	for (const auto& sc : node_path_to_classes[path])
	{
		lua_classes_vector.erase(std::remove(lua_classes_vector.begin(), lua_classes_vector.end(), sc));
	}
	node_path_to_classes.extract(path);
	object->Destroy(false);

	return 1;
}

int lua_destroy_node_by_path(std::string path)
{
	const auto& object = p_grapsh_system->GetNodeByPath(path);

//	assert(object != nullptr, "Attempt to call destroy on null object!");

	object->Destroy(false);

	return 1;
}

int lua_rotate_object_by_rotator(Node3D* object, float y, float p, float r)
{
	assert(object != nullptr, "Attempt to call rotate on null object!");
	object->Transform.Rotate(DirectX::SimpleMath::Vector3(y, p, r));

	return 1;
}

bool lua_get_ai_state(AINode* object)
{
	assert(object != nullptr, "Attempt to call get ai state on null object!");

	try
	{
		return object->IsEnabled();
	}
	catch (std::exception& ex)
	{
		return false;
	}
}

int lua_set_ai_state(AINode* object, bool state)
{
	assert(object != nullptr, "Attempt to call get ai state on null object!");

	try
	{
		object->SetEnabled(state);
		return 1;
	}
	catch (std::exception& ex)
	{
		return 1;
	}
}

sol::table lua_get_script_component_from_node(Node3D* object, const std::string& component)
{
	const std::string& path{ object->GetNodePath() };
	
	for (const auto& item : node_path_to_classes[path])
	{
		const std::string& componentPath{ item + "." + component};
		return lua[componentPath];
	}

	return sol::nil;
}

Node3D* lua_change_camera()
{
	if (Singleton::GetNodeGraph()->GetNodeByPath("root/fp_player") == Singleton::GetNodeGraph()->GetCurrentPlayer())
	{
		Singleton::GetNodeGraph()->GetNodeByPath("root/tp_player")->SetCurrent();
		return Singleton::GetNodeGraph()->GetNodeByPath("root/tp_player");
	}
	else
	{
		Singleton::GetNodeGraph()->GetNodeByPath("root/fp_player")->SetCurrent();
		return Singleton::GetNodeGraph()->GetNodeByPath("root/tp_player");
	}
}

int lua_set_object_velocity(Node3D* object, float x, float y, float z)
{
	assert(object != nullptr, "Attempt to call set velocity on null object!");

	Singleton::GetPhysicsManager()->SetObjectVelocity(object->GetNodeId(), DirectX::SimpleMath::Vector3(x, y, z));

	return 1;
}

CameraNode* lua_get_cam()
{
	return p_grapsh_system->GetCurrentCamera();
}

const std::string& lua_cast_ray(float distance)
{
	auto ray = Singleton::GetPhysicsManager()->CastRay(p_grapsh_system->GetCurrentCamera()->GetWorldPosition(), p_grapsh_system->GetCurrentCamera()->GetWorldDirection(), distance);

	if (ray.size() == 0)
	{
		return std::string{ "empty " };
	}

	return Singleton::GetNodeGraph()->GetObjectByID(ray.rbegin()->first)->GetNodePath();
}

int lua_call_assert(std::string text)
{
	std::cout << "Error: " << text << std::endl;

	return 1;
}

int lua_set_rotation_to_rotator(Node3D* object, float y, float p, float r)
{
	assert(object != nullptr, "Attempt to call rotate on null object!");

	object->Transform.SetRotation(DirectX::SimpleMath::Vector3(y, p, r));
	return 1;
}

int lua_add_impulse(Node3D* object, float x, float y, float z, float m)
{
	assert(object != nullptr, "Attempt to call add impulse on null object!");

	static_cast<PhysicalObjectNode*>(object)->AddImpulse(DirectX::SimpleMath::Vector3(x, y, z), m);
	return 1;
}

sol::table lua_get_velocity(Node3D* object)
{
	assert(object != nullptr, "Attempt to call get velocity on null object!");

	const Vector3& vec = static_cast<PhysicalObjectNode*>(object)->GetVelocity();
	return lua.create_table_with("x", vec.x,
		"y", vec.y,
		"z", vec.z);
}

sol::table lua_get_object_pos(Node3D * object)
{
	assert(object != nullptr, "Attempt to call get position on null object!");
	const auto pos = object->Transform.GetPosition();

	return lua.create_table_with(
		"x", pos.x,
		"y", pos.y,
		"z", pos.z
	);;
}

sol::table get_lua_class(std::string name)
{
	return lua[name];
}

Node3D* lua_get_child(Node3D* object, std::string childName)
{
	assert(object != nullptr, "Attempt to call get child to on null object!");

	return object->GetChild(childName);
}

int lua_change_color(Object3DNode* object, float r, float g, float b)
{
	//object->MaterialsOverride
	return 1;
}

Node3D* lua_get_parent(Node3D* object)
{
	assert(object != nullptr, "Attempt to call get parent to on null object!");

	return object->GetParent();
}

sol::table lua_get_object_world_direction(Node3D* object)
{
	assert(object != nullptr, "Attempt to call get world direction to on null object!");
	const Vector3 vec = object->GetWorldDirection();
	return lua.create_table_with(
		"x", vec.x,
		"y", vec.y,
		"x", vec.z
		);
}

int lua_transform_move_to(Node3D* object, float x, float y, float z)
{
	assert(object != nullptr, "Attempt to call move to on null object!");

	static_cast<PhysicalObjectNode*>(object)->Transform.SetScale(400);

	return 1;
}

int lua_transform_move_by(Node3D* object, float x, float y, float z)
{
	assert(object != nullptr, "Attempt to call move to on null object!");

	object->Transform.Move(DirectX::SimpleMath::Vector3(x, y, z));

	return 1;
}

int lua_load_object_with_model(std::string name)
{
	p_scene->AddObjectOnScene(name);
	std::string highCaseName = name;
	std::transform(highCaseName.begin(), highCaseName.end(), highCaseName.begin(), ::toupper);
	lua.safe_script("if " + name + " ~= nil then return end \n" + highCaseName + " = {}\n" + "Class(" + highCaseName + ", GameObject)\n" + name + " = " + highCaseName + ":new(\"" + name + "\")" +
		"\n" + name + ":SetEntityName(\"" + name + "\")\n" + name + ":Start()\n" + name + ":AddComponent(Transform)\n");

	if (lua_classes_map.find(name) == lua_classes_map.end())
	{
		std::string className = std::format("{}{}", className, lua_classes_map[name].size());
		lua_classes_map[name].emplace_back(name);
		lua_classes_vector.emplace_back(name);
	}

	return 1;
}

int lua_register_class(std::string id)
{
	for (const auto& element : lua_classes_vector)
	{
		assert(element != id, "Attempt to register class that already registered!");
	}

	lua_classes_vector.push_back(id);
	std::cout << "registered  " << id << std::endl;
	return 1;
}
//LUA API ZONE END
//////////////////////////////////////////////////////////////////////


inline bool is_regular_file_safe(const fs::directory_entry& entry, std::error_code& ec) {
	//        
#if defined(_MSC_VER)
	//  MSVC  is_regular_file   
	return fs::is_regular_file(entry.path(), ec);
#else
	//  GCC/Clang    entry
	try {
		if (entry.is_regular_file(ec)) {
			return true;
		}
		//   ,   
		return fs::is_regular_file(entry.path(), ec);
	}
	catch (...) {
		return fs::is_regular_file(entry.path(), ec);
	}
#endif
}

size_t FindAllLuaFiles(const std::string& rootPath,
	std::vector<std::string>& luaFiles,
	bool caseInsensitive = true) {
	size_t fileCount = 0;
	std::error_code ec;

	// ,   
	fs::path rootPathFs(rootPath);
	if (!fs::exists(rootPathFs, ec) || !fs::is_directory(rootPathFs, ec)) {
		std::cerr << "Error: Directory does not exist or is not accessible: "
			<< rootPath << std::endl;
		if (ec) {
			std::cerr << "System error: " << ec.message() << std::endl;
		}
		return 0;
	}

	try {
		//    
		fs::recursive_directory_iterator dirIter(rootPathFs, ec);
		if (ec) {
			std::cerr << "Error creating directory iterator for " << rootPath
				<< ": " << ec.message() << std::endl;
			return 0;
		}

		//      
		for (const auto& entry : dirIter) {
			ec.clear(); //      

			try {
				// ,     
				if (!is_regular_file_safe(entry, ec)) {
					continue;
				}

				const fs::path& filePath = entry.path();
				std::string extension = filePath.extension().string();

				//     (  )
				if (caseInsensitive) {
					std::string normalizedExt;
					normalizedExt.reserve(extension.size());
					for (char c : extension) {
						normalizedExt.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
					}
					extension = normalizedExt;
				}

				//   .lua
				if (extension == ".lua") {
					//       
					luaFiles.push_back(filePath.string());
					++fileCount;
				}
			}
			catch (const fs::filesystem_error& e) {
				std::cerr << "Warning: Could not process file "
					<< entry.path().string() << ": "
					<< e.what() << std::endl;
				continue;
			}
			catch (const std::exception& e) {
				std::cerr << "Warning: Unexpected error processing file "
					<< entry.path().string() << ": "
					<< e.what() << std::endl;
				continue;
			}
		}
	}
	catch (const fs::filesystem_error& e) {
		std::cerr << "Critical error traversing directory " << rootPath << ": "
			<< e.what() << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Critical error in file traversal: " << e.what() << std::endl;
	}

	return fileCount;
}


std::vector<std::string> FindAllLuaFiles(const std::string& rootPath,
	bool caseInsensitive = true) {
	std::vector<std::string> result;
	FindAllLuaFiles(rootPath, result, caseInsensitive);
	return result;
}

void LuaManager::LoadScrtipts()
{
	if (EngineConfig::IsReleaseMode)
	{

	}
	else
	{
		lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::math, sol::lib::os);
		lua.set_function("Register", &lua_register_class);
		lua.set_function("LoadObjectWithModel", &lua_load_object_with_model);
		lua.set_function("RotateBy", &lua_rotate_object_by_rotator);
		lua.set_function("TranslateTo", &lua_transform_move_to);
		lua.set_function("GetObjectOnScene", &lua_get_object_on_scene);
		lua.set_function("GetTransformPosition", &lua_get_object_pos);
		lua.set_function("GetClass", &get_lua_class);
		lua.set_function("TranslateBy", &lua_transform_move_by);
		lua.set_function("CallError", &lua_call_assert);
		lua.set_function("GetVelocity", &lua_get_velocity);
		lua.set_function("AddImpulse", &lua_add_impulse);
		lua.set_function("GetNodeType", &lua_get_node_type);
		lua.set_function("GetWorldDirection", &lua_get_object_world_direction);
		lua.set_function("GetChild", &lua_get_child);
		lua.set_function("GetParent", &lua_get_parent);
		lua.set_function("GetComponent", &lua_get_script_component_from_node);
		lua.set_function("GetAIState", &lua_get_ai_state);
		lua.set_function("SetAIState", &lua_set_ai_state);
		lua.set_function("GetCurrentCamera", &lua_get_cam);
		lua.set_function("CastRay", &lua_cast_ray);
		lua.set_function("ChangeCamera", &lua_change_camera);
		lua.set_function("DestroyNodeByNode", &lua_destroy_node_by_node);
		lua.set_function("DestroyNodeByNodePath", &lua_destroy_node_by_path);
		lua.set_function("SetObjectVelocity", &lua_set_object_velocity);

		fs::path currentDir = fs::current_path().parent_path().parent_path();
		std::cout << currentDir << std::endl;
		std::vector<std::string> luaFiles;
		size_t count = FindAllLuaFiles(currentDir.string(), luaFiles);

		std::cout << "\nFound " << count << " Lua files:" << std::endl;
		for (size_t i = 0; i < luaFiles.size(); ++i) {
			std::cout << "  [" << (i + 1) << "/" << count << "] " << luaFiles[i] << std::endl;
			size_t lastSlash = luaFiles[i].find_last_of("/\\");
			if (lastSlash != std::string::npos)
			{
				lua_file_classes.push_back(luaFiles[i].substr(lastSlash + 1));
			}
			else
			{
				lua_file_classes.push_back(luaFiles[i]);
			}
			lua.safe_script_file(luaFiles[i]);
		}
	}
}

void LuaManager::CallCollision(int32_t ObjectID1, uint32_t ObjectID2)
{
	Object3DNode* node_one{ p_grapsh_system->GetObjectByID(ObjectID1) };
	Object3DNode* node_two{ p_grapsh_system->GetObjectByID(ObjectID2) };

	std::string path_one = node_one != NULL ? node_one->GetNodePath() : "nil";
	std::string path_two = node_two != NULL ? node_two->GetNodePath() : "nil";

	lua["GloballCollided"](path_one, path_two);

	if (path_one != "nil")
	{
		for (const auto& sc : node_path_to_classes[path_one])
		{
			sol::table temp_class = lua[sc];
			temp_class["OnOverlap"](temp_class, path_two);
		}
	}
	
	if (path_two != "nil")
	{
		for (const auto& sc : node_path_to_classes[path_two])
		{
			sol::table temp_class = lua[sc];
			temp_class["OnOverlap"](temp_class, path_one);
		}
	}
}

void LuaManager::CallHit(int32_t ObjectID1, uint32_t ObjectID2)
{
	Object3DNode* node_one{ p_grapsh_system->GetObjectByID(ObjectID1) };
	Object3DNode* node_two{ p_grapsh_system->GetObjectByID(ObjectID2) };

	for (const auto& sc : node_path_to_classes[node_one->GetNodePath()])
	{
		sol::table temp_class = lua[sc];
		temp_class["OnHit"](temp_class, node_two->GetNodePath());
	}

	for (const auto& sc : node_path_to_classes[node_two->GetNodePath()])
	{
		sol::table temp_class = lua[sc];
		temp_class["OnHit"](temp_class, node_one->GetNodePath());
	}
}

LuaManager::LuaManager()
{
	//sol::state lua;
	LoadScrtipts();
}

LuaManager::~LuaManager()
{
}



void LuaManager::SetGraspSystem(NodeGraphSystem* system)
{
	p_grapsh_system = system;
	//const auto tuple = std::make_tuple(1, 2.5f, "dodik");
	//p_instance->CallLuaFunction("DoThing", tuple, 1);
}

void LuaManager::ProceedMouseMovementInput(MouseMotionEventArgs& e)
{
	for (const auto& lua_class : lua_classes_vector)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["OnMouseMovementInputReceived"](temp_class, e.X, e.Y);
	}
}

void LuaManager::ProceedMouseClickInput(MouseButtonEventArgs& e, bool pressed)
{
	for (const auto& lua_class : lua_classes_vector)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["OnMouseClickInput"](temp_class, e, pressed);
	};
}

void LuaManager::ProceedMouseWheelInput(MouseWheelEventArgs& e)
{
	for (const auto& lua_class : lua_classes_vector)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["OnMouseWheelInput"](temp_class, e);
	};
}

void LuaManager::ProceedKeyBoardInput(KeyCode::Key& e, bool pressed)
{
	for (const auto& lua_class : lua_classes_vector)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["OnKeyBoardInput"](temp_class, e, pressed);
	};
}

void LuaManager::PerformUpdate()
{
	for (const auto& lua_class : lua_classes_vector)
	{
		if (lua_class != "")
		{
			sol::table temp_class = lua[lua_class];
			temp_class["Update"](temp_class);
		}
	}
}

void LuaManager::Start()
{
	for (const auto& lua_class : lua_classes_vector)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["Start"](temp_class);
	}
}

std::string LuaManager::CreateValidClass(std::string className, std::string objId, NodeTypeEnum type)
{
	if (lua_classes_map.find(className) == lua_classes_map.end())
	{
		lua_classes_map.insert({ className, std::vector<std::string>() });
	}

	int index = lua_classes_map[className].size();
	std::string actualName = std::format("{}{}", className, index);
	lua_classes_map[className].emplace_back(actualName);
	std::string highCaseName = className;
	std::transform(highCaseName.begin(), highCaseName.end(), highCaseName.begin(), ::toupper);
	std::string components = "";
	if (type >= NodeTypeEnum::NODE_TYPE_NODE3D)
	{
		components += actualName + ":AddComponent(Transform)\n";
	}

	if (type >= NodeTypeEnum::NODE_TYPE_OBJECT3D)
	{

	}

	if (type >= NodeTypeEnum::NODE_TYPE_PHYSICAL_OBJECT3D)
	{
		components += actualName + ":AddComponent(Physics)\n";
	}
	lua.safe_script("if " + actualName + " ~= nil then return end \n" + actualName + " = " + highCaseName + ":new(\"" + actualName + "\")" +
		"\n" + actualName + ":SetEntityName(\"" + objId + "\")\n" + components);

	if (node_path_to_classes.find(objId) == node_path_to_classes.end())
	{
		node_path_to_classes.insert({ objId, std::vector<std::string>{actualName} });
	}
	else
	{
		node_path_to_classes[objId].push_back(actualName);
	}

	lua_register_class(actualName);
	return actualName;
}

void LuaManager::ReloadScripts()
{
	lua_classes_vector.clear();
	lua_file_classes.clear();
	lua_classes_map.clear();
	lua.~state();
	lua = sol::state();
}

void LuaManager::StartScript(std::string className)
{
	lua_register_class(className);
	sol::table temp_class = lua[className];
	temp_class["Start"](temp_class);
}

void LuaManager::UpdateScript(std::string script)
{
	sol::table temp_class = lua[script];
	temp_class["Update"](temp_class);
}


std::vector<std::string>& LuaManager::GetAllFoundScriptClasses()
{
	return lua_file_classes;
}

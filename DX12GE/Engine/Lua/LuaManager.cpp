#include <iostream>
#include <typeinfo>
#include <string>
#include "../SingleGpuGame.h"
#include <vector>
#include <cctype>
#include <filesystem>
#include "../DX12GE/EngineConfig.h"
#include <algorithm>
#include <system_error>
#include <direct.h> 
#include "LuaManager.h"
//#define SOL_ALL_SAFETIES_ON 0

namespace fs = std::filesystem;

#define TEST

lua_State* LuaManager::L = nullptr;
LuaManager* LuaManager::p_instance = nullptr;
std::string const luaSciptsFolder = "../DX12GE/Engine/Lua/scripts/";
static SingleGpuGame* p_scene;
static NodeGraphSystem* p_grapsh_system;
static std::vector<std::string> lua_classes_vector;
static std::map<std::string, std::vector<std::string>> lua_classes_map;
static sol::state lua;

//////////////////////////////////////////////////////////////////////
//LUA API ZONE
Node3D* lua_get_object_on_scene(std::string name)
{
	const auto object = p_grapsh_system->GetNodeByPath(name);
	//object->AddScriptComponent();

	return object;
}

int lua_rotate_object_by_rotator(Node3D* object, float y, float p, float r)
{
	assert(object != nullptr, "Attempt to call rotate on null object!");
	object->Transform.SetRotation(DirectX::SimpleMath::Vector3(y, p, r));

	return 1;
}

std::map<std::string, float> lua_get_object_pos(Node3D* object)
{
	assert(object != nullptr, "Attempt to call get position on null object!");
	const auto pos = object->Transform.GetPosition();
	std::map<std::string, float> posMap;
	posMap.insert({ "x", pos.x });
	posMap.insert({ "y", pos.y });
	posMap.insert({ "z", pos.z });
	return posMap;
}


int lua_set_camera_target(lua_State* L)
{
	const auto& target = static_cast<Node3D*>(lua_touserdata(L, 1));
	const Vector3 objectPos = target->Transform.GetPosition() + Vector3(0.0f, 2.0f, 0.0f);

	const float x = (float)lua_tonumber(L, 2);
	const float y = (float)lua_tonumber(L, 3);
	const float z = (float)lua_tonumber(L, 4);
	const float flyRadius = (float)lua_tonumber(L, 5);

	const Vector3 camPosition(objectPos + Vector3(x, y, z) * flyRadius);
	//p_camera->Position = XMVectorSet(camPosition.x, camPosition.y, camPosition.z, 1.0f);

	Vector3 razn = objectPos - camPosition;
	razn.Normalize();
	//p_camera->Target = XMVectorSet(razn.x , razn.y, razn.z, 1.0f);

	return 1;
}

sol::table get_lua_class(std::string name)
{
	return lua[name];
}

int lua_transform_move_to(Node3D* object, float x, float y, float z)
{
	assert(object != nullptr, "Attempt to call move to on null object!");

	object->Transform.SetPosition(DirectX::SimpleMath::Vector3(x, y, z));

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

LuaManager::LuaManager()
{
	//sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io);

	lua.set_function("Register", &lua_register_class);
	lua.set_function("LoadObjectWithModel", &lua_load_object_with_model);
//	lua.set_function("GetCamera", &lua_get_camera);
	lua.set_function("RotateBy", &lua_rotate_object_by_rotator);
	lua.set_function("TranslateTo", &lua_transform_move_to);
	lua.set_function("GetObjectOnScene", &lua_get_object_on_scene);
	lua.set_function("GetTransfromPosition", &lua_get_object_pos);
	lua.set_function("GetClass", &get_lua_class);
	lua.set_function("TranslateBy", &lua_transform_move_by);

	fs::path currentDir = fs::current_path().parent_path().parent_path();
	std::cout << currentDir << std::endl;
	std::vector<std::string> luaFiles;
	size_t count = FindAllLuaFiles(currentDir.string(), luaFiles);

	std::cout << "\nFound " << count << " Lua files:" << std::endl;
	for (size_t i = 0; i < luaFiles.size(); ++i) {
		std::cout << "  [" << (i + 1) << "/" << count << "] " << luaFiles[i] << std::endl;
		lua.safe_script_file(luaFiles[i]);
	}

	if (!EngineConfig::IsReleaseMode)
	{
		lua.safe_script_file(luaSciptsFolder + "Core.lua");
		lua.safe_script_file(luaSciptsFolder + "Player.lua");
		lua.safe_script_file(luaSciptsFolder + "TestScript.lua");
		lua.safe_script_file(luaSciptsFolder + "TestScript2.lua");
	}
	else
	{
		lua.safe_script_file(luaSciptsFolder + "Core.lua");
		lua.safe_script_file(luaSciptsFolder + "Player.lua");
		//	lua.safe_script_file("TestScript.lua");
			//lua.safe_script_file("TestScript2.lua");
	}
	

	//*/

}

LuaManager::~LuaManager()
{
}

///////////////////////////////////////////////////////////////// ABOUT TO BE CUTTED
bool LuaManager::CheckLua(lua_State* L, int r)
{
	if (r != LUA_OK)
	{
		const std::string errormsg = lua_tostring(L, -1);
		std::cout << errormsg << std::endl;
		return false;
	}
	return true;
}

void LuaManager::LoadErrorStack()
{
	const std::string error = lua_tostring(L, -1);
	std::cout << error << std::endl;
}

std::string LuaManager::ReadStringFromTable(std::string tableName, std::string keyName)
{
	lua_getglobal(L, tableName.c_str());

	if (!lua_istable(L, -1))
	{
		std::cout << "No Such string: " << keyName << "in table" << std::endl;
		return "error";
	}

	lua_pushstring(L, keyName.c_str());
	lua_gettable(L, -2);
	const std::string res = lua_tostring(L, -1);
	lua_pop(L, 1);
	return res;
}

long double LuaManager::ReadNumderFromTable(std::string tableName, std::string keyName)
{
	lua_getglobal(L, tableName.c_str());

	if (!lua_istable(L, -1))
	{
		std::cout << "No Such number: " << keyName << "in table" << std::endl;
		return -1;
	}

	lua_pushstring(L, keyName.c_str());
	lua_gettable(L, -2);
	const long double res = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return res;
}

bool LuaManager::ReadBoolFromTable(std::string tableName, std::string keyName)
{
	lua_getglobal(L, tableName.c_str());

	if (!lua_istable(L, -1))
	{
		std::cout << "No Such bool: " << keyName << "in table" << std::endl;
		return false;
	}

	lua_pushstring(L, keyName.c_str());
	lua_gettable(L, -2);
	const bool res = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return res;
}

template<typename... Args>
void LuaManager::CallLuaFunction(std::string functionName, const std::tuple<Args...>& inputs, const int ouputsCount)
{
	lua_getglobal(L, functionName.c_str());

	if (lua_isfunction(L, -1))
	{
		std::apply([this](const Args&... args) {
			(PushToLua(args), ...);
			}, inputs);

		if (lua_pcall(L, sizeof...(Args), ouputsCount, 0) != LUA_OK) {
			const char* error = lua_tostring(L, -1);
			std::cerr << "Lua error: " << error << std::endl;
			lua_pop(L, 1);
		}
	}
	else
	{
		lua_pop(L, 1);
		std::cerr << "Function '" << functionName << "' not found" << std::endl;
	}
}

template<typename T>
void LuaManager::PushToLua(const T& value)
{
	if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
		lua_pushinteger(L, value);
	}
	else if constexpr (std::is_floating_point_v<T>) {
		lua_pushnumber(L, value);
	}
	else if constexpr (std::is_same_v<T, bool>) {
		lua_pushboolean(L, value);
	}
	else if constexpr (std::is_convertible_v<T, std::string>) {
		lua_pushstring(L, std::string(value).c_str());
	}
	else
	{
		const void* val = &value;
		lua_pushlightuserdata(L, const_cast<void*>(val));
	}
}


std::string LuaManager::ReadUserDataFromTable(std::string tableName, std::string keyName)
{
	return std::string();
}
///////////////////////////////////////////////////////////////// ABOUT TO BE CUTTED END ZONE


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
		sol::table temp_class = lua[lua_class];
		temp_class["Update"](temp_class);
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


std::string LuaManager::CreateValidClass(std::string className, std::string objId)
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
	lua.safe_script("if " + actualName + " ~= nil then return end \n" + actualName + " = " + highCaseName + ":new(\"" + actualName + "\")" +
		"\n" + actualName + ":SetEntityName(\"" + objId + "\")\n" + actualName + ":AddComponent(Transform)\n");

	lua_register_class(actualName);
	return actualName;
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

#include <iostream>
#include <typeinfo>
#include <string>
#include "SingleGpuGame.h"
#include "LuaManager.h"
#define SOL_ALL_SAFETIES_ON 0


lua_State* LuaManager::L = nullptr;
LuaManager* LuaManager::p_instance = nullptr;
std::string const luaSciptsFolder = "../Lua/scripts/";
static SingleGpuGame* p_scene;
static std::vector<std::string> lua_classes;
static sol::state lua;
static Camera* p_camera;

//////////////////////////////////////////////////////////////////////
//LUA API ZONE
Object3DEntity* lua_get_object_on_scene(std::string name)
{
	const auto object = p_scene->Get(name);
	return object;
}

int lua_rotate_object_by_rotator(Object3DEntity* object, float y, float p, float r)
{
	assert(object != nullptr, "Attemp to call rotate on null object!");
	object->Transform.SetRotation(DirectX::SimpleMath::Vector3(y, p, r));

	return 1;
}

Camera* lua_get_camera() 
{
	return p_camera;
}

int lua_set_camera_target(lua_State* L)
{
	const auto& target = static_cast<Object3DEntity*>(lua_touserdata(L, 1));
	const Vector3 objectPos = target->Transform.GetPosition() + Vector3(0.0f, 2.0f, 0.0f);

	const float x = (float)lua_tonumber(L, 2);
	const float y = (float)lua_tonumber(L, 3);
	const float z = (float)lua_tonumber(L, 4);
	const float flyRadius = (float)lua_tonumber(L, 5);

	const Vector3 camPosition(objectPos + Vector3(x, y, z) * flyRadius);
	p_camera->Position = XMVectorSet(camPosition.x, camPosition.y, camPosition.z, 1.0f);

	Vector3 razn = objectPos - camPosition;
	razn.Normalize();
	p_camera->Target = XMVectorSet(razn.x , razn.y, razn.z, 1.0f);

	return 1;
}

int lua_transform_move_to(Object3DEntity* object, float x, float y, float z)
{
	assert(object != nullptr, "Attemp to call move to on null object!");

	object->Transform.SetPosition(DirectX::SimpleMath::Vector3(x, y, z));

	return 1;
}

int lua_load_object_with_model(std::string path)
{
	shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();
	return 1;
}

int lua_register_class(std::string id)
{
	for (const auto& element : lua_classes)
	{
		assert(element != id, "Attemp to register class that already registered!");
	}

	lua_classes.push_back(id);
	std::cout << "registered  " << id << std::endl;
	return 1;
}
//LUA API ZONE END
//////////////////////////////////////////////////////////////////////

LuaManager::LuaManager()
{
	//sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io);
	
	lua.set_function("Register", &lua_register_class);
	lua.set_function("LoadObjectWithModel", &lua_load_object_with_model);
	lua.set_function("GetCamera", &lua_get_camera);
	lua.set_function("RotateBy", &lua_rotate_object_by_rotator);
	lua.set_function("TranslateTo", &lua_transform_move_to);
	lua.set_function("GetObjectOnScene", &lua_get_object_on_scene);

	lua.safe_script_file(luaSciptsFolder + "Core.lua");
	lua.safe_script_file(luaSciptsFolder + "Player.lua");
	lua.safe_script_file(luaSciptsFolder + "TestScript.lua");
	lua.safe_script_file(luaSciptsFolder + "TestScript2.lua");
	
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


void LuaManager::SetScene(SingleGpuGame* scene)
{
	p_scene = scene;
	const auto tuple = std::make_tuple(1, 2.5f, "dodik");
	//p_instance->CallLuaFunction("DoThing", tuple, 1);
}

void LuaManager::ProceedMouseMovementInput(MouseMotionEventArgs& e)
{
	for (const auto& lua_class : lua_classes)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["OnMouseMovementInputReceived"](temp_class, e.X, e.Y);
	};;
}

void LuaManager::ProceedMouseClickInput(MouseButtonEventArgs& e, bool pressed)
{
	for (const auto& lua_class : lua_classes)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["OnMouseClickInput"](temp_class, e, pressed);
	};
}

void LuaManager::ProceedMouseWheelInput(MouseWheelEventArgs& e)
{
	for (const auto& lua_class : lua_classes)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["OnMouseWheelInput"](temp_class, e);
	};
}

void LuaManager::ProceedKeyBoardInput(KeyEventArgs& e, bool pressed)
{
	for (const auto& lua_class : lua_classes)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["OnKeyBoardInput"](temp_class, e, pressed);
	};
}

void LuaManager::PerformUpdate()
{
	for (const auto& lua_class : lua_classes)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["Update"](temp_class);
	}
}

void LuaManager::Start()
{
	for (const auto& lua_class : lua_classes)
	{
		sol::table temp_class = lua[lua_class];
		temp_class["Start"](temp_class);
	}
}

void LuaManager::SetCamera(Camera* camera)
{
	p_camera = camera;
}



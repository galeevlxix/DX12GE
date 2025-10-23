#include <iostream>
#include <typeinfo>
#include <string>
#include "SingleGpuGame.h"
#include "LuaManager.h"

lua_State* LuaManager::L = nullptr;
LuaManager* LuaManager::p_instance = nullptr;
std::string const luaSciptsFolder = "../Lua/scripts/";
static SingleGpuGame* p_scene;
static Camera* p_camera;

//////////////////////////////////////////////////////////////////////
//LUA API ZONE
int lua_Foo(lua_State* L)
{
	const float a = (float)lua_tonumber(L, 1);
	const float b = (float)lua_tonumber(L, 2);
	std::cout << "Foo called" << std::endl;
	const float c = a * b;
	lua_pushnumber(L, c);
	return 1;
}

int lua_get_object_on_scene(lua_State* L)
{
	std::string name = lua_tostring(L, 1);
	const auto object = p_scene->Get(name);
	lua_pushlightuserdata(L, object);

	return 1;
}

int lua_move_object_to_position(lua_State* L)
{
	const auto& object = static_cast<Object3DEntity*>(lua_touserdata(L, 1));
	const float x = (float)lua_tonumber(L, 2);
	const float y = (float)lua_tonumber(L, 3);
	const float z = (float)lua_tonumber(L, 4);
	object->Transform.SetPosition(DirectX::SimpleMath::Vector3(x, y, z));

	return 1;
}

int lua_rotate_object_by_rotator(lua_State* L)
{
	const auto& object = static_cast<Object3DEntity*>(lua_touserdata(L, 1));
	const float y = (float)lua_tonumber(L, 2);
	const float p = (float)lua_tonumber(L, 3);
	const float r = (float)lua_tonumber(L, 4);

	object->Transform.SetRotation(DirectX::SimpleMath::Vector3(y, p, r));

	return 1;
}

int lua_get_camera(lua_State* L) 
{
	lua_pushlightuserdata(L, p_camera);
	return 1;
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
//LUA API ZONE END
//////////////////////////////////////////////////////////////////////

LuaManager::LuaManager()
{
	L = luaL_newstate();
	luaL_openlibs(L);
	const int r = luaL_dofile(L, (luaSciptsFolder + "TestScript.lua").c_str());


	if (CheckLua(L, r))
	{
		lua_register(L, "Foo", lua_Foo);
		lua_register(L, "get_object_on_scene", lua_get_object_on_scene);
		lua_register(L, "move_object_to_position", lua_move_object_to_position);
		lua_register(L, "rotate_object_by_rotator", lua_rotate_object_by_rotator);
		lua_register(L, "get_camera", lua_get_camera);
		lua_register(L, "set_camera_target", lua_set_camera_target);
	}
}

LuaManager::~LuaManager()
{
}

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

void LuaManager::SetScene(SingleGpuGame* scene)
{
	p_scene = scene;
	const auto tuple = std::make_tuple(1, 2.5f, "dodik");
	p_instance->CallLuaFunction("DoThing", tuple, 1);
}

void LuaManager::ProceedMouseMovementInput(MouseMotionEventArgs& e)
{
	const auto tuple = std::make_tuple(e.X, e.Y);
	p_instance->CallLuaFunction("OnMouseMovementInputReceived", tuple, 1);
}

void LuaManager::ProceedMouseClickInput(MouseButtonEventArgs& e, bool pressed)
{
	const auto tuple = std::make_tuple(e, pressed);
	p_instance->CallLuaFunction("OnMouseClickInputReceived", tuple, 1);
}

void LuaManager::ProceedMouseWheelInput(MouseWheelEventArgs& e)
{
	const auto tuple = std::make_tuple(e);
	p_instance->CallLuaFunction("OnMouseWheelInputReceived", tuple, 1);
}

void LuaManager::ProceedKeyBoardInput(KeyEventArgs& e, bool pressed)
{
	const auto tuple = std::make_tuple(e, pressed);
	p_instance->CallLuaFunction("OnKeyBoardInputReceived", tuple, 1);
}

void LuaManager::PerformUpdate()
{
	const std::string name = "Update";
	lua_getglobal(L, name.c_str());
	lua_pcall(L, 0, 0, 0);
}

void LuaManager::SetCamera(Camera* camera)
{
	p_camera = camera;
}



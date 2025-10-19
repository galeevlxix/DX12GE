#include <iostream>
#include <typeinfo>
#include <string>
#include "LuaManager.h"

lua_State* LuaManager::L = nullptr;
LuaManager* LuaManager::p_instance = nullptr;
std::string const luaSciptsFolder = "../Lua/scripts/";


LuaManager::LuaManager()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	int r = luaL_dofile(L, (luaSciptsFolder + "TestScript.lua").c_str());

	if (CheckLua(L, r))
	{
		auto tuple = std::make_tuple(1, 2.5f, "dodik");
		CallLuaFunction("DoThing", tuple, 1);
	}
}

LuaManager::~LuaManager()
{
}

bool LuaManager::CheckLua(lua_State* L, int r)
{
	if (r != LUA_OK)
	{
		std::string errormsg = lua_tostring(L, -1);
		std::cout << errormsg << std::endl;
		return false;
	}
	return true;
}

void LuaManager::LoadErrorStack()
{
	std::string error = lua_tostring(L, -1);
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
	std::string res = lua_tostring(L, -1);
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
	long double res = lua_tonumber(L, -1);
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
	bool res = lua_toboolean(L, -1);
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
	else {
		static_assert(sizeof(T) == 0, "Unsupported type for Lua push");
	}
}


std::string LuaManager::ReadUserDataFromTable(std::string tableName, std::string keyName)
{
	return std::string();
}



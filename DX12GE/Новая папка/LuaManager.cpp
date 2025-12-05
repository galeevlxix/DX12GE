#include <iostream>
#include <string>
#include "LuaManager.h"

lua_State* LuaManager::p_luaState = nullptr;
LuaManager* LuaManager::p_instance = nullptr;
std::string const luaSciptsFolder = "scripts/";

LuaManager::LuaManager()
{
	p_luaState = luaL_newstate();
	luaL_openlibs(p_luaState);
	std::string cmd = "a = 7 + 22  + math.sin(23.7)";

	int r = luaL_dofile(p_luaState, (luaSciptsFolder + "TestScript.lua").c_str());

	if (IsValid(r))
	{
		lua_getglobal(p_luaState, "a");
		if (lua_isnumber(p_luaState, -1))
		{
			float result = (float)lua_tonumber(p_luaState, -1);
			std::cout << result << std::endl;
		}
	}
}

LuaManager::~LuaManager()
{
}

bool LuaManager::IsValid(int r)
{
	if (r == LUA_OK)
	{
		return true;
	}
	else
	{
		LoadErrorStack();
		return false;
	}
}

void LuaManager::LoadErrorStack()
{
	std::string error = lua_tostring(p_luaState, -1);
	std::cout << error << std::endl;
}



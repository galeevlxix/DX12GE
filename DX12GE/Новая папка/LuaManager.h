#pragma once

extern "C"
{
#include "include/lua.h"
#include "include/lauxlib.h"
#include "include/lualib.h"
}

class LuaManager
{
private:
	LuaManager();
	virtual ~LuaManager();
	bool IsValid(int r);
	void LoadErrorStack();
	static lua_State* p_luaState;
	static LuaManager* p_instance;

public:
	static LuaManager* GetInstance() {
		if (p_instance == nullptr)
		{
			p_instance = new LuaManager();
		}
		return p_instance;
	};

	static void CLose() {
		lua_close(p_luaState);
	}
};


#pragma once

extern "C"
{
#include "include/lua.h"
#include "include/lauxlib.h"
#include "include/lualib.h"
}

#define SCRIPT "../Lua/scripts/"
#include <vector>

class LuaManager
{
public:
	enum LuaTypes
	{
		Number,
		String,
		Boolean,
		UserData,
		Table,
		Function
	};

	struct LuaValueType
	{
		LuaValueType(intptr_t* dataPtr, LuaTypes type) : dataPtr(dataPtr), type(type) {};

		intptr_t* dataPtr;
		LuaTypes type;
	};
private:
	LuaManager();
	virtual ~LuaManager();
	void LoadErrorStack();
	bool CheckLua(lua_State* L, int r);
	std::string ReadStringFromTable(std::string tableName, std::string keyName);
	long double ReadNumderFromTable(std::string tableName, std::string keyName);
	bool ReadBoolFromTable(std::string tableName, std::string keyName);

	template<typename... Args>
	void CallLuaFunction(std::string functionName, const std::tuple<Args...>& tuple1, const int ouputsCount);

	template<typename T>
	void PushToLua(const T& value);
	std::string ReadUserDataFromTable(std::string tableName, std::string keyName);
	static lua_State* L;
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
		lua_close(L);
	}
};


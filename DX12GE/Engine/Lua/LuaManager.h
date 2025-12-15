#pragma once
#include "../SingleGpuGame.h"
#include "sol/sol.hpp"
#include "sol/config.hpp"
#include "sol/forward.hpp"

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

	static void SetScene(SingleGpuGame* scene);
	static void ProceedMouseMovementInput(MouseMotionEventArgs&);
	static void ProceedMouseClickInput(MouseButtonEventArgs& e, bool pressed);
	static void ProceedMouseWheelInput(MouseWheelEventArgs& e);
	static void ProceedKeyBoardInput(KeyEventArgs& e, bool pressed);
	static void PerformUpdate();
	static void Start();
	static void SetCamera(Camera* camera);
	static void CLose() {
		lua_close(L);
	}
	static std::string CreateValidClass(std::string className, std::string objId);
	static void StartScript(std::string className);
	static void UpdateScript(std::string script);
};


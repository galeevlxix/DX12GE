#pragma once
#include "../DX12GE/Engine/NodeGraph/NodeGraphSystem.h"
#include "Lua/sol/sol.hpp"
#include "Lua/sol/config.hpp"
#include "Lua/sol/forward.hpp"
#include "../DX12GE/Engine/NodeGraph/NodeTypeEnum.h"

extern "C"
{
#include "Lua/include/lua.h"
#include "Lua/include/lauxlib.h"
#include "Lua/include/lualib.h"
}

#define SCRIPT "../Lua/scripts/"
#include <vector>

class LuaManager
{
private:
	LuaManager();
	virtual ~LuaManager();

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

	static void SetGraspSystem(NodeGraphSystem* system);
	static void ProceedMouseMovementInput(MouseMotionEventArgs&);
	static void ProceedMouseClickInput(MouseButtonEventArgs& e, bool pressed);
	static void ProceedMouseWheelInput(MouseWheelEventArgs& e);
	static void ProceedKeyBoardInput(KeyCode::Key& e, bool pressed);
	static void PerformUpdate();
	static void Start();
	static void CLose() {
		lua_close(L);
	}
	static std::string CreateValidClass(std::string className, std::string objId, NodeTypeEnum type);
	static void StartScript(std::string className);
	static void ReloadScripts();
	static void UpdateScript(std::string script);
	static std::vector<std::string>& GetAllFoundScriptClasses();
	static void LoadScrtipts();
	static void CallCollision(int32_t ObjectID1, uint32_t ObjectID2);
	static void CallHit(int32_t ObjectID1, uint32_t ObjectID2);
	//static void CallCollision(std::string ObjectID1, std::string ObjectID2);
};


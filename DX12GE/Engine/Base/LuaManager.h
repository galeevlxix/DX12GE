#pragma once
#include "../DX12GE/Engine/NodeGraph/NodeGraphSystem.h"
#include "../DX12GE/Engine/NodeGraph/NodeTypeEnum.h"
#include "Lua/sol/sol.hpp"
#include <vector>

class LuaManager
{
private:
	LuaManager();
	virtual ~LuaManager();

	static lua_State* L;
	static LuaManager* p_instance;
public:
	static LuaManager* GetInstance();

	static void SetGraspSystem(NodeGraphSystem* system);
	static void ProceedMouseMovementInput(MouseMotionEventArgs&);
	static void ProceedMouseClickInput(MouseButtonEventArgs& e, bool pressed);
	static void ProceedMouseWheelInput(MouseWheelEventArgs& e);
	static void ProceedKeyBoardInput(KeyCode::Key& e, bool pressed);
	static void PerformUpdate();
	static void Start();
	static void CLose();
	static std::string CreateValidClass(std::string className, std::string objId, NodeTypeEnum type);
	static void StartScript(std::string className);
	static void ReloadScripts();
	static void UpdateScript(std::string script);
	static std::vector<std::string>& GetAllFoundScriptClasses();
	static void LoadScrtipts();
	static void CallCollision(int32_t ObjectID1, uint32_t ObjectID2);
	static void CallHit(int32_t ObjectID1, uint32_t ObjectID2);
};


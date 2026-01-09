#pragma once

#include "SceneJsonSerializer.h"
#include "CommandExecutor.h"
#include "SelectionSystem.h"
#include "../Graphics/DebugRenderSystem.h"
#include "../NodeGraph/NodeGraphSystem.h"
#include "../Graphics/CurrentPass.h"
#include "AudioSystem.h"
#include "../DX12GE/Engine/Physics/PhysicsManager.h"

using namespace Physics;

class Singleton
{
public:
	static void Initialize();
	static void Destroy();

	static CommandExecutor* GetExecutor();
	static DebugRenderSystem* GetDebugRender();
	static SelectionSystem* GetSelection();
	static NodeGraphSystem* GetNodeGraph();
	static SceneJsonSerializer* GetSerializer();
	static CurrentPass* GetCurrentPass();
	static AudioSystem* GetAudioSystem();
	static PhysicsManager* GetPhysicsManager();

	static bool IsInitialized();
};
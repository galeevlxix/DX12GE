#pragma once

#include "SceneJsonSerializer.h"
#include "CommandExecutor.h"
#include "SelectionSystem.h"
#include "../Graphics/DebugRenderSystem.h"
#include "../NodeGraph/NodeGraphSystem.h"
#include "../Graphics/CurrentPass.h"
#include "AudioSystem.h"

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

	static bool IsInitialized();
};
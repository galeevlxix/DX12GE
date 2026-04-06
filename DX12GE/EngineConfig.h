#pragma once
#include <map>

enum class EngineConfigRuntimeOutput
{
	RUNTIME_OUTPUT_UNKNOWN,
	RUNTIME_OUTPUT_EDITOR,
	RUNTIME_OUTPUT_WINDOW
};

enum class EngineConfigRuntimeMode
{
	RUNTIME_MODE_UNKNOWN,
	RUNTIME_MODE_EDITING,
	RUNTIME_MODE_PLAYING
};

class EngineConfig
{
public:
	static EngineConfigRuntimeOutput Output;
	static EngineConfigRuntimeMode Mode;
};

static const std::map<const char*, EngineConfigRuntimeOutput> RuntimeOutputMap = 
{
	{"editor", EngineConfigRuntimeOutput::RUNTIME_OUTPUT_EDITOR},
	{"window", EngineConfigRuntimeOutput::RUNTIME_OUTPUT_WINDOW}
};

static const std::map<const char*, EngineConfigRuntimeMode> RuntimeModeMap =
{
	{"editing", EngineConfigRuntimeMode::RUNTIME_MODE_EDITING},
	{"edit", EngineConfigRuntimeMode::RUNTIME_MODE_EDITING},
	{"playing", EngineConfigRuntimeMode::RUNTIME_MODE_PLAYING},
	{"play", EngineConfigRuntimeMode::RUNTIME_MODE_PLAYING},
};
#pragma once
#include "DirectX12Viewport.h"

class EngineController
{
public:
	static int Run(DirectX12Viewport* viewport);
	static void Quit();
};


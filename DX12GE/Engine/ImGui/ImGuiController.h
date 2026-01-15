#pragma once

#include "../Base/DX12LibPCH.h"
#include "../NodeGraph/Node3D.h"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

class ImGuiController
{
public:
	static void Create(HWND hWnd);
	static void OnRenderStart();
	static void OnRenderEnd(double deltaTime, ComPtr<ID3D12GraphicsCommandList2> commandList);

	static bool CursorOnWindow();
	static void ShutDown();

private:

	static bool AddVector3Edit(const char* label, Vector3& value);
	static bool AddColor3Edit(const char* label, Vector3& value);

	static void UpdateSceneTree(Node3D* node);
	static void UpdateInspector(Node3D* node);

	static void DrawDirContents();
};
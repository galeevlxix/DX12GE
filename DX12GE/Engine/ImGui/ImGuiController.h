#pragma once

#include "../Base/DX12LibPCH.h"
#include "../NodeGraph/Node3D.h"
#include "../Base/Window.h"
#include "../Graphics/TextureBuffer.h"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

class ImGuiController
{
public:
	static void Create(std::shared_ptr<Window> window);
	static void OnRenderStart();
	static void OnRenderEnd(double deltaTime, ComPtr<ID3D12GraphicsCommandList2> commandList, std::shared_ptr<TextureBuffer> tb);

	static bool CursorOnWindow();
	static void ShutDown();

	static void ChangeVisiblity();

private:

	static bool AddVector3Edit(const char* label, Vector3& value);
	static bool AddColor3Edit(const char* label, Vector3& value);

	static void UpdateSceneTree(Node3D* node);
	static void UpdateInspector(Node3D* node);

	static void UpdateTexture(const std::string& label, uint32_t id, TextureType type);
	static void UpdateMaterialInspector(Node3D* node);

	static void DrawDirContents();
};
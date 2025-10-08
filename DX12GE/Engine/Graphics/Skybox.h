#pragma once

#include "Mesh3DComponent.h"
#include "TextureComponent.h"
#include "TransformComponent.h"

class Skybox
{
	Mesh3DComponent m_BoxMesh;
	TextureComponent m_Texture;
	TransformComponent m_Transform;

public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix);
	void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot);
	void Destroy();
};
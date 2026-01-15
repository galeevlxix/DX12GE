#pragma once

#include "Object3DNode.h"
#include "../Graphics/Mesh3DComponent.h"

/// \brief Skybox class.
/// \note Only one such node in the scene tree can be active. 
class SkyBoxNode : public Object3DNode
{
protected:
	/// \brief Texture component id in the resource storage.
	uint32_t m_TextureId;

public:
	SkyBoxNode();

	/// \brief Loads the texture and creates a skybox mesh.
	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath) override;

	/// \brief Skybox rendering.
	virtual void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix) override;

	/// \brief Rendering skybox textures.
	void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot);

	virtual void Destroy(bool keepComponent = true) override;

	virtual void SetComponentId(uint32_t newId) override;

	virtual uint32_t GetComponentId() override { return m_TextureId; }

	virtual bool IsValid() override { return m_TextureId != -1; }

	virtual const std::string GetObjectFilePath() override;

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;

	/// \brief Checks whether this skybox is active in the scene.
	/// \return Returns true if this skybox is current. Returns false otherwise.
	bool IsCurrent();

private:
	bool TreeHasSkyboxesWithComponentId(uint32_t id, Node3D* current = nullptr);
	static void CreateBoxMesh(ComPtr<ID3D12GraphicsCommandList2> commandList);

public:
	static void DestroyBoxMesh();
};	
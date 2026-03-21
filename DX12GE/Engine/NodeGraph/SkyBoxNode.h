#pragma once

#include "Object3DNode.h"
#include "../Graphics/Mesh3DComponent.h"

// Класс узла неба (skybox)
class SkyBoxNode : public Object3DNode
{
protected:
	Mesh3DComponent m_BoxMesh;
	uint32_t m_TextureId;

public:
	SkyBoxNode();

	// Загружает текстуру и создает мэш скайбокса
	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath) override;

	// Отрисовка скайбокса
	virtual void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix) override;

	// Отрисовка текстуры скайбокса
	void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot);

	// Уничтожает данный узел и всех потомков узла
	virtual void Destroy(bool keepComponent = true) override;

	virtual void SetComponentId(uint32_t newId) override;

	virtual uint32_t GetComponentId() override { return m_TextureId; }

	virtual bool IsValid() override { return m_TextureId != -1; }

	virtual const std::string GetObjectFilePath() override;

	virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;
	bool IsCurrent();

private:
	// Возвращает true, если в дереве существует узел с таким же id компонента
	// Если root определен, обход дерева начинается с него
	bool TreeHasSkyboxesWithComponentId(uint32_t id, Node3D* current = nullptr);
};	
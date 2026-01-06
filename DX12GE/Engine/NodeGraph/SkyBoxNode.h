#pragma once

#include "Object3DNode.h"
#include "../Graphics/Mesh3DComponent.h"

// ����� ���� ���� (skybox)
class SkyBoxNode : public Object3DNode
{
protected:
	Mesh3DComponent m_BoxMesh;
	uint32_t m_TextureId;

public:
	SkyBoxNode();

	// ��������� �������� � ������� ��� ���������
	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath, const std::string& nodePath) override;

	// ��������� ���������
	virtual void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix) override;

	// ��������� �������� ���������
	void RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot);

	// ���������� ������ ���� � ���� �������� ����
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
	// ���������� true, ���� � ������ ���������� ���� � ����� �� id ����������
	// ���� root ���������, ����� ������ ���������� � ����
	bool TreeHasSkyboxesWithComponentId(uint32_t id, Node3D* current = nullptr);
};	
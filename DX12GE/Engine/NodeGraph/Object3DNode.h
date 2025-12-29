#pragma once

#include "Node3D.h"
#include "../Base/DX12LibPCH.h"
#include "../Base/CollisionBox.h"

// ����� ���� 3� �������
class Object3DNode : public Node3D
{
protected:
	uint32_t m_ComponentId;
	
	std::vector<VertexStruct> Vertices;
	std::vector<WORD> Indices;

public:
	bool IsVisible;

	Object3DNode();

	// ��������� ������ ����
	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath);

	// ��������� 3� �������
	virtual void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix);
	
	// ���������� ������ ���� � ���� �������� ����
	// ���� ����� ��������� �� ������ �������� ������ ��������
	// ��� keepComponent = false �� ������ ����� ��������� ��������� � �������, ���� � ������ ����� ��� ������ �����, ������������ ���� ��������� (���������������� �� ��������)
	virtual void Destroy(bool keepComponent = true) override;
	
	// ������������� ����� ��������� ������� (������ � �������) �� id ��� ������� ����
	virtual void SetComponentId(uint32_t newId);

	virtual void UpdateTransform(DirectX::SimpleMath::Matrix InTransform = DirectX::SimpleMath::Matrix());
	
	// ���������� id ���������� (������� � �������) � ResourceStorage
	virtual uint32_t GetComponentId() { return m_ComponentId; }

	virtual bool IsValid() { return m_ComponentId != -1; }

	virtual const std::string GetObjectFilePath();
	const CollisionBox& GetCollisionBox();
	
	virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;
	
	vector<VertexStruct> GetVertices() { return Vertices; };
	
	vector<WORD> GetIndices() { return Indices; };

private:
	// ���������� true, ���� � ������ ���������� ���� � ����� �� id ����������
	// ���� root ���������, ����� ������ ���������� � ����
	bool TreeHasObjects3DWithComponentId(uint32_t id, Node3D* current = nullptr);
};

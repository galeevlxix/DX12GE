#pragma once

#include "Node3D.h"
#include "../Base/DX12LibPCH.h"
#include "../Base/CollisionBox.h"

<<<<<<< HEAD
/// \brief 3D object node class.
=======
// ����� ���� 3� �������
>>>>>>> master
class Object3DNode : public Node3D
{
protected:

	// \brief ID of the 3D object component in the resource storage.
	uint32_t m_ComponentId;

public:
	bool IsVisible;

	Object3DNode();

<<<<<<< HEAD
	/// \brief Loads node data.
	/// \param filePath The path to the file that stores data about meshes, vertices, and indices. For example, .obj or .gltf.
	/// \return Returns true if creation was successful. Returns false otherwise.
	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath);

	/// \brief 3D object rendering.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	virtual void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix);
	
	virtual void Destroy(bool keepComponent = true) override;
	
	/// \brief Sets a new object component (resource with data) by id for this node.
	virtual void SetComponentId(uint32_t newId);

	/// \brief Returns the id of a component (resource with data) in the resource storage.
=======
	// ��������� ������ ����
	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath, const std::string& nodePath);

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
>>>>>>> master
	virtual uint32_t GetComponentId() { return m_ComponentId; }

	/// \brief Checks whether a 3D object is valid for rendering.
	/// \return Returns false if the creation of the 3D object was not successful or if the 3D object is broken. Returns true otherwise.
	virtual bool IsValid() { return m_ComponentId != -1; }

	/// \brief Returns the data file of a 3D object. 
	virtual const std::string GetObjectFilePath();

	/// \brief Returns AABB of object 3d component
	/// \throws std::runtime_error If this object 3d node is invalid
	const CollisionBox& GetCollisionBox();
	
	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

private:
<<<<<<< HEAD
=======
	// ���������� true, ���� � ������ ���������� ���� � ����� �� id ����������
	// ���� root ���������, ����� ������ ���������� � ����
>>>>>>> master
	bool TreeHasObjects3DWithComponentId(uint32_t id, Node3D* current = nullptr);
};

#pragma once

#include "../Graphics/TransformComponent.h"
#include "NodeSerializingData.h"
#include "../Base/Events.h"
#include <map>
#include <vector>

// ������� ����� ���� � ������ �����
class Node3D
{
public:
	TransformComponent Transform;

protected:
	std::map<std::string, Node3D*> m_Children;
	Node3D* m_Parrent = nullptr;
	std::string m_Name;
	DirectX::XMMATRIX m_WorldMatrixCache;

	DirectX::SimpleMath::Vector3 m_WorldPositionCache;
	DirectX::SimpleMath::Vector3 m_WorldDirectionCache;

	NodeTypeEnum m_Type;

public:
	Node3D();

	// ���������� ��� ����
	NodeTypeEnum GetType() { return m_Type; }

	// ���������� ���� ���� � ������ ����� �� ������ �������� �������� �� ������� ����
	const std::string GetNodePath();
	const std::string GetNodeScripts();
	

	// ?????????? ??? ???????? ????
	virtual void OnLoad() {};

	
	// ��������� ������ ���� � ���� �������� ����
	virtual void OnUpdate(const double& deltaTime);

	// ���������� ������ ���� � ���� �������� ����
	// ���� ����� ��������� �� ������ �������� ������ ��������
	// ��� keepComponent = false �� ������ ����� ��������� ��������� � �������, ���� � ������ ����� ��� ������ �����, ������������ ���� ��������� (keepComponent ���������������� �� ��������)
	virtual void Destroy(bool keepComponent = true);
    
	// ���������� ������� ������� �������
	// ����������� ������ ����� OnUpdate
	const DirectX::XMMATRIX& GetWorldMatrix();

	// ���������� ������� ������� � ������� ������������
	// ����������� ������ ����� OnUpdate
	const DirectX::SimpleMath::Vector3& GetWorldPosition() { return m_WorldPositionCache; }

	// ���������� ����������� ������� � ������� ������������
	// ����������� ������ ����� OnUpdate
	const DirectX::SimpleMath::Vector3& GetWorldDirection() { return m_WorldDirectionCache; }

	// ���������� ��� ����
	const std::string GetName() { return m_Name; }
	
	// ��������������� ������ ����
	// ���� ����� ������ �������� �������� ����� ���� ��� ����������� ������� � ����� ������, � ������ ����� ����������� �����, ����� �������� ���������� ���� ����� �������� ��������
	// ��� �� ������ ��������� �������: '/', '%'
	void Rename(const std::string& name);

	// ���������� true, ���� ���� ��������� � ������ �����
	bool IsInsideTree();

	// ���������� ������� �������
	Node3D* GetChild(const std::string& name);

	// ���������� ��� �������� ���� 
	// ��������! ������ ����� ����� ���� ���������!
	const std::vector<Node3D*> GetChildren();

	// ���������� ����� ���� �������� �����
	// ��������! ������ ����� ����� ���� ���������!
	const std::vector<std::string> GetChildrenNames();

	// ���������� ������� ��������
	Node3D* GetParrent();

	// ������� ������ ����, ��� ��� ��������� � name
	// ��������! ������ ����� ����� ���� ���������!
	Node3D* FindNodeRecursive(const std::string& name);

	// ������� ������� ��������, ��� ��� ��������� � name
	Node3D* FindParrentRecursive(const std::string& name);

	// ��������� � ������ ���� ������ ������� �������
	// ���������� true, ���� ���������� ������ �������
	// ��������! ������� �� ���������, ���� � ���� ��� ���� ��������. ����������� ����� Move � ���� ������.
	virtual bool AddChild(Node3D* node);

	// ������� ���� �� ������ ������ �������� �� �����, �� �� ���������� ������ �������
	// ���������� true, ���� �������� ������ �������
	bool RemoveChild(const std::string& name);

	// ������� ���� �� ������ ������ ��������, �� �� ���������� ������ �������
	// ���������� true, ���� �������� ������ �������
	bool RemoveChild(Node3D* node);

	// ���������� true, ���� ������� ������ ������� � ����� ������
	bool HasChild(const std::string& name);

	// ���������� ������ ���� � ���� ������ ��������
	// ���������� true, ���� ����������� ������ �������
	bool Move(Node3D* newParrent);

	// ��������� ������ ����, ������� ������ � ��������� � cloneNode
	// ������ �������������:
	//		Node3D* newNode = oldNode->Clone(someParrent, true);
	// ���� parrent ���������, ����� ���� ��������� � ���� ����� ��������
	// ���� cloneChildrenRecursive ����� true, ��� ������� ����� ������������
	// ���������� nullptr, ���� ������������ �� ������ �������
	// ��������! ������ ����� ����� ���� ���������!
	virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr);

	// ������ ���������� ���������
	virtual void DrawDebug();

	// ������ ���� ������������ �������� �� ����� (�-� CameraNode, DirectionalLight, Environment � ��)
	virtual void SetCurrent() {};

	// ������� JSON ������ ��� ������������ ����
	virtual void CreateJsonData(json& j);

	// ��������� ������ ���� �� ��������� NodeSerializingData
	virtual void LoadFromJsonData(const NodeSerializingData& nodeData);

private:
	void NotifyParrentChanged();
	void RenameChild(const std::string& oldName, const std::string& newName);
	
public:

	virtual void OnKeyPressed(KeyEventArgs& e);
	virtual void OnKeyReleased(KeyEventArgs& e);
	virtual void OnMouseWheel(MouseWheelEventArgs& e);
	virtual void OnMouseMoved(MouseMotionEventArgs& e);
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
	virtual void OnWindowResize(ResizeEventArgs& e);
};
#pragma once

#include "Node3D.h"
#include "../Base/DX12LibPCH.h"
#include "../Base/CollisionBox.h"

// Класс узла 3Д объекта
class Object3DNode : public Node3D
{
protected:
	uint32_t m_ComponentId = -1;

public:
	bool IsVisible;

	Object3DNode();

	// Загружает данные узла
	virtual bool Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath);

	// Отрисовка 3Д объекта
	void Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix);
	
	// Уничтожает данный узел и всех потомков узла
	// Узел также удаляется из списка потомков своего родителя
	// При keepComponent = false из памяти также удаляется компонент с данными, если в дереве сцены нет других узлов, использующих этот компонент (распространяется на потомков)
	virtual void Destroy(bool keepComponent = true) override;
	
	// Устанавливает новый компонент объекта (ресурс с данными) по id для данного узла
	virtual void SetComponentId(uint32_t newId);

	// Возвращает id компонента (ресурса с данными) в ResourceStorage
	uint32_t GetComponentId() { return m_ComponentId; }

	bool IsValid() { return m_ComponentId != -1; }

	const std::string GetObjectFilePath();
	const CollisionBox& GetCollisionBox();
	
	virtual void Clone(Node3D* cloneNode, Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

private:
	// Возвращает true, если в дереве существует узел с таким же id компонента
	// Если root определен, обход дерева начинается с него
	bool TreeHasObjects3DWithComponentId(uint32_t id, Node3D* current = nullptr);
};

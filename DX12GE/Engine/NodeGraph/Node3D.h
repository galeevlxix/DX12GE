#pragma once

#include "../Graphics/TransformComponent.h"
#include "NodeSerializingData.h"
#include "../Base/Events.h"
#include <map>
#include <vector>

// Базовый класс узла в дереве сцены
class Node3D
{
public:
	TransformComponent Transform;

protected:
	std::map<std::string, Node3D*> m_Children;
	Node3D* m_Parrent = nullptr;
	std::string m_Name;
    uint32_t m_ComponentId = -1;
	DirectX::XMMATRIX m_WorldMatrixCache;

	DirectX::SimpleMath::Vector3 m_WorldPositionCache;
	DirectX::SimpleMath::Vector3 m_WorldDirectionCache;

	NodeTypeEnum m_Type;

public:
	Node3D();

	// Возвращает тип узла
	NodeTypeEnum GetType() { return m_Type; }

	// Возвращает путь узла в дереве сцены от самого дальнего родителя до данного узла
	const std::string GetNodePath();
	
	// Обновляет данный узел и всех потомков узла
	virtual void OnUpdate(const double& deltaTime);

	// Уничтожает данный узел и всех потомков узла
	// Узел также удаляется из списка потомков своего родителя
	// При keepComponent = false из памяти также удаляется компонент с данными, если в дереве сцены нет других узлов, использующих этот компонент (keepComponent распространяется на потомков)
	virtual void Destroy(bool keepComponent = true);
    
	// Возвращает мировую матрицу объекта
	// Обновляется только после OnUpdate
	const DirectX::XMMATRIX& GetWorldMatrix();

	// Возвращает позицию объекта в мировом пространстве
	// Обновляется только после OnUpdate
	const DirectX::SimpleMath::Vector3& GetWorldPosition() { return m_WorldPositionCache; }

	// Возвращает направление объекта в мировом пространстве
	// Обновляется только после OnUpdate
	const DirectX::SimpleMath::Vector3& GetWorldDirection() { return m_WorldDirectionCache; }

	// Возвращает id компонента (ресурса с данными) в ResourceStorage
	uint32_t GetComponentId() { return m_ComponentId; }

	// Устанавливает новый компонент (ресурс с данными) по id для данного узла
	virtual void SetComponentId(uint32_t newId) { m_ComponentId = newId; }

	// Возвращает имя узла
	const std::string GetName() { return m_Name; }
	
	// Переименовывает данный узел
	// Если среди прямых потомков родителя этого узла уже встречается потомок с таким именем, к новому имени добавляется число, чтобы избежать повторения имен среди потомков родителя
	// Имя не должно содержать символы: '/', '%'
	void Rename(const std::string& name);

	// Возвращает true, если узел находится в дереве сцены
	bool IsInsideTree();

	// Возвращает прямого потомка
	Node3D* GetChild(const std::string& name);

	// Возвращает все дочерние узлы 
	// Внимание! Данный метод может быть медленным!
	const std::vector<Node3D*> GetChildren();

	// Возвращает имена всех дочерних узлов
	// Внимание! Данный метод может быть медленным!
	const std::vector<std::string> GetChildrenNames();

	// Возвращает прямого родителя
	Node3D* GetParrent();

	// Находит первый узел, чье имя совпадает с name
	// Внимание! Данный метод может быть медленным!
	Node3D* FindNodeRecursive(const std::string& name);

	// Находит первого родителя, чье имя совпадает с name
	Node3D* FindParrentRecursive(const std::string& name);

	// Добавляет в данный узел нового прямого потомка
	// Возвращает true, если добавление прошло успешно
	// Внимание! Потомок не добавится, если у него уже есть родитель. Используйте метод Move в этом случае.
	virtual bool AddChild(Node3D* node);

	// Удаляет узел из списка прямых потомков по имени, но не уничтожает самого потомка
	// Возвращает true, если удаление прошло успешно
	bool RemoveChild(const std::string& name);

	// Удаляет узел из списка прямых потомков, но не уничтожает самого потомка
	// Возвращает true, если удаление прошло успешно
	bool RemoveChild(Node3D* node);

	// Возвращает true, если нашелся прямой потомок с таким именем
	bool HasChild(const std::string& name);

	// Перемещает данный узел в узел нового родителя
	// Возвращает true, если перемещение прошло успешно
	bool Move(Node3D* newParrent);

	// Клонирует данный узел, копируя данные и параметры в cloneNode
	// Пример использования:
	//		Node3D* newNode;
	//		oldNode->Clone(newNode, someParrent);
	// Если parrent определен, новый узел добавится в узел этого родителя
	// Если cloneChildrenRecursive равен true, все потомки также склонируются
	// Возвращает nullptr, если клонирование не прошло успешно
	// Внимание! Данный метод может быть медленным!
	virtual void Clone(Node3D* cloneNode, Node3D* parrent = nullptr, bool cloneChildrenRecursive = false);

	// Рисует отладочные примитивы
	virtual void DrawDebug();

	// Делает узел единственным активным на сцене (н-р CameraNode, DirectionalLight, Environment и тд)
	virtual void SetCurrent() {};

	// Создает JSON данные для сериализации узла
	virtual void CreateJsonData(json& j);

	// Загружает данные узла из структуры NodeSerializingData
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
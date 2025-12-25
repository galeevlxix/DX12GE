#pragma once

#include "FirstPersonPlayerNode.h"

//  ласс узла игрока дл€ управлени€ камерой от третьего лица
// “олько один такой узел в дереве сцены может быть активным 
class ThirdPersonPlayerNode : public FirstPersonPlayerNode
{
public:
	float MinFlyRadius;
	float MaxFlyRadius;
	Vector3 CameraAnchor;

protected:
	float m_FlyRadius;

public:
	ThirdPersonPlayerNode();

	virtual void OnUpdate(const double& deltaTime) override;

	virtual void Destroy(bool keepComponent = true) override;

	virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
};
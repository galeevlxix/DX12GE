#pragma once

#include "Node3D.h"
  
// Класс узла камеры
// Только один такой узел в дереве сцены может быть активным 
class CameraNode : public Node3D
{
protected:
	Vector3 m_Up;
	float m_Ratio;

public:	

	float Fov;
	float ZNear;
	float ZFar;

	CameraNode();

	const Matrix GetViewProjMatrix();
	const Matrix GetViewProjMatrixNoTranslation();

	virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;
	bool IsCurrent();

	virtual void OnWindowResize(ResizeEventArgs& e) override;

	void SetRatio(float ratio) { m_Ratio = ratio; }
};
#pragma once

#include "Node3D.h"
  
// Класс камеры
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

	virtual void Clone(Node3D* cloneNode, Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false) override;

	virtual void OnWindowResize(ResizeEventArgs& e) override;

	void SetRatio(float ratio) { m_Ratio = ratio; }
};
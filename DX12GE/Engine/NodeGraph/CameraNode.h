#pragma once

#include "Node3D.h"
  
/// \brief Camera node class.
/// \note Only one such node in the scene tree can be active. 
class CameraNode : public Node3D
{
protected:
	/// \brief Camera Up vector. Usually equal to (0, 1, 0).
	Vector3 m_Up;

	/// \brief Application window aspect ratio.
	float m_Ratio;

public:	

	/// \brief The angle of view of the camera in degrees.
	float Fov;

	/// \brief Near clipping plane of the camera. Minimum view distance.
	float ZNear;
	
	/// \brief Far clipping plane of the camera. Maximum view distance.
	float ZFar;

	CameraNode();

	/// \brief Returns a matrix that combines the view matrix and the projection matrix. Used to display objects.
	const Matrix GetViewProjMatrix();

	/// \brief Returns a matrix that combines the no-trasnslation-view matrix and the projection matrix. Used to display the skybox.
	const Matrix GetViewProjMatrixNoTranslation();

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;

	/// \brief Checks whether this camera is active in the scene.
	/// \return Returns true if this camera is current. Returns false otherwise.
	bool IsCurrent();

	virtual void OnWindowResize(ResizeEventArgs& e) override;

	/// \brief Changes application window aspect ratio for the camera.
	/// \note Called automatically by the engine if the node is in the scene tree. No need to call it explicitly.
	void SetRatio(float ratio) { m_Ratio = ratio; }
};
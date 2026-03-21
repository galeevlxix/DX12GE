#pragma once
#include "../Base/DX12LibPCH.h"

#include "Node3D.h"

/// \brief Node class for listening to sounds in a scene. 
/// \note Only one such node in the scene tree can be active. 
class AudioListenerNode : public Node3D
{
	X3DAUDIO_LISTENER m_ListenerData;
	Vector3 m_PrevWorldPosition;

public:
	AudioListenerNode();

	virtual void OnUpdate(const double& deltaTime) override;

	/// \brief Returns a point of 3D audio reception.
	const X3DAUDIO_LISTENER& GetListenerData() { return m_ListenerData; }

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;

	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	/// \brief Checks whether this listener is active in the scene.
	/// \return Returns true if this listener is current. Returns false otherwise.
	bool IsCurrent();
};

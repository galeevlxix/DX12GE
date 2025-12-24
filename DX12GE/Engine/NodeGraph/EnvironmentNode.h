#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

// Класс узла окружения сцены (окружающий свет, туман, эффекты постобработки и тд)
// Только один такой узел в дереве сцены может быть активным 
class EnvironmentNode : public Node3D
{
public:
	BaseLightComponent AmbientLightData;

	bool FogEnabled;
	DirectX::SimpleMath::Vector3 FogColor;
	float FogStart;
	float FogDistance;

	float SSRMaxDistance;
	float SSRStepLength;
	float SSRThickness;

	EnvironmentNode();

	virtual Node3D* Clone(Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;

	virtual void DrawDebug() override;

	virtual void CreateJsonData(json& j) override;

	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

	virtual void SetCurrent() override;
	bool IsCurrent();
};
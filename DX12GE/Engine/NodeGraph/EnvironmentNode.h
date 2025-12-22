#pragma once

#include "Node3D.h"
#include "../Graphics/GraphicsComponents.h"

// Общие параметры графики
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

	virtual void Clone(Node3D* cloneNode, Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false) override;

	virtual void DrawDebug() override;

	virtual void SetCurrent() override;
	bool IsCurrent();
};
#pragma once

#include "Node3D.h"

// Общие параметры графики
// Может быть только один такой узел в дереве сцены
class EnvironmentNode : public Node3D
{
public:
	DirectX::SimpleMath::Vector3 AmbientLightColor;
	float AmbientLightIntensity;

	bool FogEnabled;
	DirectX::SimpleMath::Vector3 FogColor;
	float FogStart;
	float FogDistance;

	float SSRMaxDistance;
	float SSRStepLength;
	float SSRThickness;

	EnvironmentNode();

	virtual const std::string GetType() override { return "EnvironmentNode"; }

	virtual void Clone(Node3D* cloneNode, Node3D* newParrent = nullptr, bool cloneChildrenRecursive = false) override;

};
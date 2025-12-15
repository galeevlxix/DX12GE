#include "../EnvironmentNode.h"

EnvironmentNode::EnvironmentNode() : Node3D()
{
	AmbientLightColor = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);
	AmbientLightIntensity = 0.2f;

	FogEnabled = true;
	FogColor = DirectX::SimpleMath::Vector3(0.5f, 0.5f, 0.5f);
	FogStart = 35.0f;
	FogDistance = 230.0f;

	SSRMaxDistance = 32.0f;
	SSRStepLength = 0.1f;
	SSRThickness = 0.0999f;
	
	Rename("EnvironmentNode");
}

void EnvironmentNode::Clone(Node3D* cloneNode, Node3D* newParrent, bool cloneChildrenRecursive)
{
	if (!cloneNode)
	{
		cloneNode = new EnvironmentNode();
	}

	Node3D::Clone(cloneNode, newParrent, cloneChildrenRecursive);

	if (cloneNode)
	{
		EnvironmentNode* env = dynamic_cast<EnvironmentNode*>(cloneNode);

		env->AmbientLightColor = AmbientLightColor;
		env->AmbientLightIntensity = AmbientLightIntensity;

		env->FogEnabled = FogEnabled;
		env->FogColor = FogColor;
		env->FogStart = FogStart;
		env->FogDistance = FogDistance;

		env->SSRMaxDistance = SSRMaxDistance;
		env->SSRStepLength = SSRStepLength;
		env->SSRThickness = SSRThickness;
	}
}

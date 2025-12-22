#pragma once

#include "FirstPersonPlayerNode.h"

class ThirdPersonPlayerNode : public FirstPersonPlayerNode
{
protected:
	float m_FlyRadius = 5.0f;

public:
	ThirdPersonPlayerNode();

	virtual void OnUpdate(const double& deltaTime) override;
	virtual void Destroy(bool keepComponent = true) override;

	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
};
#pragma once
#include <string>
#include "NodeTypeEnum.h"
#include "../Base/SimpleMath.h"

#include "../Base/json.hpp"
using json = nlohmann::json;

struct NodeSerializingData
{
	std::string nodePath;
	NodeTypeEnum type;
	std::string filePath;

	DirectX::SimpleMath::Vector3 pos;
	DirectX::SimpleMath::Vector3 rot;
	DirectX::SimpleMath::Vector3 scl;

	bool isCurrent = false;
	bool isVisible = false;

	// lights
	DirectX::SimpleMath::Vector3 lightColor;
	float lightIntensity;
	DirectX::SimpleMath::Vector3 lightAttenuation;
	float lightCutoff;

	//environment
	bool envFogEnabled;
	DirectX::SimpleMath::Vector3 envFogColor;
	float envFogStart;
	float envFogDistance;

	float envSSRMaxDistance;
	float envSSRStepLength;
	float envSSRThickness;

	// camera
	float camFov;
	float camZNear;
	float camZFar;

	// player
	float MouseSensitivity;
	float WheelSensitivity;
	float MinMovementSpeed;
	float NormalMovementSpeed;
	float MaxMovementSpeed;

	float MinFlyRadius;
	float MaxFlyRadius;
	DirectX::SimpleMath::Vector3 CameraAnchor;
};
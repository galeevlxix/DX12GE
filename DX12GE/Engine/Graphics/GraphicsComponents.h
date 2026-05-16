#pragma once

#include "../Base/SimpleMath.h"

#define BIAN_COLOR_WHITE	DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f)
#define BIAN_COLOR_BLACK	DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f)
#define BIAN_COLOR_GRAY		DirectX::SimpleMath::Vector3(0.5f, 0.5f, 0.5f)
							
#define BIAN_COLOR_RED		DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f)
#define BIAN_COLOR_ORANGE	DirectX::SimpleMath::Vector3(1.0f, 0.5f, 0.0f)
#define BIAN_COLOR_YELLOW	DirectX::SimpleMath::Vector3(1.0f, 1.0f, 0.0f)
#define BIAN_COLOR_GREEN	DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f)
#define BIAN_COLOR_CYAN		DirectX::SimpleMath::Vector3(0.0f, 0.5f, 1.0f)
#define BIAN_COLOR_BLUE		DirectX::SimpleMath::Vector3(0.0f, 0.0f, 1.0f)
#define BIAN_COLOR_PURPLE	DirectX::SimpleMath::Vector3(0.5f, 0.0f, 1.0f)

#define BIAN_PINK_COLOR		DirectX::SimpleMath::Vector3(0.54902f, 0.0f,	 0.8549f)
#define BIAN_BRIGHT_COLOR	DirectX::SimpleMath::Vector3(1.0f,	   0.87059f, 0.71373f)

struct BaseLightComponent
{
	// Light source color
	DirectX::SimpleMath::Vector3 Color;

	// Light source intensity
	float Intensity;

	BaseLightComponent(DirectX::SimpleMath::Vector3 color, float intensity) :
		Color(color), 
		Intensity(intensity) { }

	BaseLightComponent(BaseLightComponent* bLight) :
		Color(bLight->Color),
		Intensity(bLight->Intensity) { }

	BaseLightComponent() :
		Color(BIAN_COLOR_WHITE),
		Intensity(0.2f) { }
};

struct DirectionalLightComponent
{
	// Base light properties for this light source
	BaseLightComponent BaseLightProperties;

	// Read-only! Direction of the light source in world space
	// To change the direction of the light source, use the Transform of the node
	DirectX::SimpleMath::Vector4 WorldDirection;

	DirectionalLightComponent() : 
		BaseLightProperties({ BIAN_COLOR_WHITE, 1.0f }),
		WorldDirection(DirectX::SimpleMath::Vector4(1.0f, -1.0f, -1.0f, 1.0f )) { }

	DirectionalLightComponent(DirectionalLightComponent* dLight) :
		BaseLightProperties(dLight->BaseLightProperties),
		WorldDirection(DirectX::SimpleMath::Vector4(1.0f, -1.0f, -1.0f, 1.0f)) { }
};

struct AttenuationComponent
{
	float Constant;
	float Linear;
	float Exp;

	AttenuationComponent() :
		Constant(1.0f),
		Linear(0.09f),
		Exp(0.032f) { }

	AttenuationComponent(AttenuationComponent* atten) :
		Constant(atten->Constant),
		Linear(atten->Linear),
		Exp(atten->Exp) { }
};

// Point light source component
struct PointLightComponent
{
	// Base light properties for this light source
	BaseLightComponent BaseLightProperties;

	// Read-only! Position of the light source in world space
	// To change the position of the light source, use the Transform of the node
	DirectX::SimpleMath::Vector3 WorldPosition;

	// Attenuation properties of the light source
	AttenuationComponent AttenuationProperties;

	PointLightComponent() :
		BaseLightProperties({ BIAN_COLOR_WHITE, 1.0f }),
		WorldPosition(DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f)),
		AttenuationProperties(AttenuationComponent())
	{ }

	PointLightComponent(PointLightComponent* pLight) :
		BaseLightProperties(pLight->BaseLightProperties), 
		WorldPosition(DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f)),
		AttenuationProperties(pLight->AttenuationProperties) { }
};

// Spot light source component
struct SpotLightComponent
{
	// Base light properties for this spot light source
	PointLightComponent PointLightProperties;

	// Read-only! Direction of the light source in world space
	// To change the direction of the light source, use the Transform of the node
	DirectX::SimpleMath::Vector3 WorldDirection;

	// Angle at which the spot light source ends
	float Cutoff;

	SpotLightComponent() :
		PointLightProperties(PointLightComponent()),
		WorldDirection(DirectX::SimpleMath::Vector3(0.0f, 0.0f, -1.0f)),
		Cutoff(0.65f) { }

	SpotLightComponent(SpotLightComponent *sLight) : 
		PointLightProperties(sLight->PointLightProperties),
		WorldDirection(DirectX::SimpleMath::Vector3(0.0f, 0.0f, -1.0f)),
		Cutoff(sLight->Cutoff) { }
};
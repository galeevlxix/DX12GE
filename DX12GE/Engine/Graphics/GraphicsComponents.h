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
	// Цвет источника света
	DirectX::SimpleMath::Vector3 Color = BIAN_COLOR_WHITE;

	// Интенсивность источника света
	float Intensity = 0.2f;

	BaseLightComponent(DirectX::SimpleMath::Vector3 color, float intensity) :
		Color(color), 
		Intensity(intensity) { }

	BaseLightComponent() :
		Color(BIAN_COLOR_WHITE),
		Intensity(0.2f) { }
};

struct DirectionalLightComponent
{
	// Свойства базового света для этого источника света
	BaseLightComponent BaseLightProperties = BaseLightComponent(BIAN_COLOR_WHITE, 1.0f);

	// Только для чтения! Направление источника света в мировом пространстве
	// Для изменения направления источника света, используйте Transform узла
	DirectX::SimpleMath::Vector4 Direction = { 1.0f, -1.0f, -1.0f, 1.0f };
};

struct AttenuationComponent
{
	float Constant = 1.0f;
	float Linear = 0.09f;
	float Exp = 0.032f;
};

// Компонент всенаправленного источника света
struct PointLightComponent
{
	// Свойства базового света для этого источника света
	BaseLightComponent BaseLightProperties = BaseLightComponent(BIAN_COLOR_WHITE, 1.0f);

	// Только для чтения! Позиция источника света в мировом пространстве
	// Для изменения позиции источника света, используйте Transform узла
	DirectX::SimpleMath::Vector3 WorldPosition = { 0.0f, 0.0f, 0.0f };

	// Параметры затухания источника света
	AttenuationComponent AttenuationProperties;
};

// Компонент прожекторного источника света
struct SpotLightComponent
{
	// Свойства всенаправленного света для этого прожекторного источника света
	PointLightComponent PointLightProperties;

	// Только для чтения! Направление источника света в мировом пространстве
	// Для изменения направления источника света, используйте Transform узла
	DirectX::SimpleMath::Vector3 WorldDirection = { 0.0f, 0.0f, -1.0 };

	// Угол, на котором свет прожекторного источника заканчивается
	float Cutoff = 0.65f;
};
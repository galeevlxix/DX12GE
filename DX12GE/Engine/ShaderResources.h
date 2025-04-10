#pragma once

#include "DX12LibPCH.h"
#include "SimpleMath.h"
#include "UploadBuffer.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

const int CASCADES_COUNT = 4;

// LIGHTS

struct BaseLight
{
	Vector3 Color;
	float Intensity;
};

struct DirectionalLight
{
	BaseLight BaseLightComponent;
	Vector4 Direction;
};

struct LightProperties
{
	Vector4 CameraPos;
	float SpecularIntensity;
	float MaterialPower;
	float PointLightsCount;
	float SpotlightsCount;
};

struct Attenuation
{
	float Constant;
	float Linear;
	float Exp;
};

struct PointLight
{
	BaseLight BaseLightComponent;
	Vector3 Position;
	Attenuation AttenuationComponent;
};

struct SpotLight
{
	PointLight PointLightComponent;
	Vector3 Direction;
	float Cutoff;
};

// CONSTANT BUFFERS

struct ObjectConstantBuffer
{
	Matrix WorldViewProjection;
	Matrix ModelViewProjection;
};

struct ShadowConstantBuffer
{
	Matrix ShadowTransforms[CASCADES_COUNT];
};

struct WorldConstantBuffer
{
	BaseLight AmbientLight;
	DirectionalLight DirLight;
	LightProperties LightProps;
};

class ShaderResourceBuffers
{
public:
	static void Create();
	static ObjectConstantBuffer* GetObjectCB();
	static ShadowConstantBuffer* GetShadowCB();
	static WorldConstantBuffer* GetWorldCB();
	static UploadBuffer* GetUploadBuffer();
	static void OnDelete();

	static void SetGraphicsObjectCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
	static void SetGraphicsShadowCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
	static void SetGraphicsWorldCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
};

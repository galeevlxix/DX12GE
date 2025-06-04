#pragma once

#include "DX12LibPCH.h"
#include "SimpleMath.h"
#include "UploadBuffer.h"
#include <vector>

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

const int CASCADES_COUNT = 4;
const int GBUFFER_COUNT = 5;

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
	float MaxRadius;
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

	Matrix ViewProjection;
};

struct ParticleConstantBuffer
{
	Matrix ViewProjection;
	Vector4 CameraPosition;
};

struct ParticleComputeConstantBuffer
{
	Vector3 BoxPosition;
	float DeltaTime;
	Vector3 BoxSize;
	float ParticleCount;
};

struct BitonicSortConstantBuffer
{
	UINT Level;
	UINT LevelMask;
	UINT ParticleCount;
	UINT Pad;
	Vector4 CameraPos;
};

struct MaterialConstantBuffer
{
	Vector4 HasDiffuseNormalEmissive;
	Vector4 HasOcclusionRoughnessMetallicCombined;
};

class ShaderResources
{
public:
	static void Create();
	static ObjectConstantBuffer* GetObjectCB();
	static ShadowConstantBuffer* GetShadowCB();
	static WorldConstantBuffer* GetWorldCB();
	static ParticleConstantBuffer* GetParticleCB();
	static ParticleComputeConstantBuffer* GetParticleComputeCB();
	static BitonicSortConstantBuffer* GetBitonicSortCB();
	static MaterialConstantBuffer* GetMaterialCB();

	static UploadBuffer* GetUploadBuffer();
	
	static void OnDelete();

	static void SetGraphicsObjectCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
	static void SetGraphicsShadowCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
	static void SetGraphicsWorldCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
	static void SetGraphicsParticleCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
	static void SetParticleComputeCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
	static void SetBitonicSortCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
	static void SetMaterialCB(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot);
};

template<typename T>
static void SetGraphicsDynamicStructuredBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, const vector<T>& bufferData)
{
	size_t bufferSize = bufferData.size() * sizeof(T);
	size_t size = sizeof(T);
	auto heapAllocation = ShaderResources::GetUploadBuffer()->Allocate(bufferSize, size);
	memcpy(heapAllocation.CPU, bufferData.data(), bufferSize);
	commandList->SetGraphicsRootShaderResourceView(slot, heapAllocation.GPU);
}

// Set buffer data of floats
template<typename T>
static void SetGraphicsConstants(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, const T& bufferData)
{
	UINT size = sizeof(T);
	commandList->SetGraphicsRoot32BitConstants(slot, size / 4, &bufferData, 0);
}

// Transition a resource
static void TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);
	commandList->ResourceBarrier(1, &barrier);
}
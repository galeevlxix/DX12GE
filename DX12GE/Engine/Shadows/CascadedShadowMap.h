#pragma once

#include "ShadowMap.h"
#include "DirectXCollision.h"
#include "../SimpleMath.h"
#include "../../Game/BianObject.h"
#include "../ShaderResources.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

struct Cascade
{
	ShadowMap* ShadowMapTexture = nullptr;
	float Radius = 0.0;
	Matrix ShadowViewProj;
};

class CascadedShadowMap
{
private:
	Cascade m_Cascades[CASCADES_COUNT];
	
	const float maxRadius = 200.0;
	const UINT width = 2048;
	const UINT height = 2048;

	void ApplyShadowTransforms();

public:
	void Create();
	void Update(Vector3 centerPosition, Vector4 dirLightDirection);
	void SetGraphicsRootDescriptorTables(int fromSlot, ComPtr<ID3D12GraphicsCommandList2> commandList);
	
	ShadowMap* GetShadowMap(int index);
	Matrix GetShadowViewProj(int index);
};
#include "CascadedShadowMap.h"
#include "../Application.h"
#include "../DescriptorHeaps.h"

void CascadedShadowMap::Create()
{
	ComPtr<ID3D12Device2> device = Application::Get().GetDevice();

	const float splits[CASCADES_COUNT] = { 0.15f, 0.3f, 0.6f, 1.0f };

	LPCWSTR names[] =	//for debug
	{
		L"Shadow Map 0",
		L"Shadow Map 1",
		L"Shadow Map 2",
		L"Shadow Map 3"
	};

	for (int i = 0; i < CASCADES_COUNT; i++)
	{
		m_Cascades[i].ShadowMapTexture = new ShadowMap(device, width, height, i);
		m_Cascades[i].ShadowMapTexture->Resource()->SetName(names[i]);
		m_Cascades[i].Radius = sqrtf(maxRadius * splits[i] * maxRadius * splits[i] * 2.0);
	}
}

void CascadedShadowMap::Update(Vector3 centerPosition, Vector4 dirLightDirection)
{
	static const Vector3 up = Vector3(0.0f, 1.0f, 0.0f);

	for (int i = 0; i < CASCADES_COUNT; i++)
	{
		Vector3 lightPos = centerPosition - m_Cascades[i].Radius * dirLightDirection;
		Matrix lightView = XMMatrixLookAtLH(lightPos, centerPosition, up);
		Vector3 sphereCenterLS = XMVector3TransformCoord(centerPosition, lightView);
		
		float l = sphereCenterLS.x - m_Cascades[i].Radius;
		float b = sphereCenterLS.y - m_Cascades[i].Radius;
		float n = sphereCenterLS.z - m_Cascades[i].Radius;
		float r = sphereCenterLS.x + m_Cascades[i].Radius;
		float t = sphereCenterLS.y + m_Cascades[i].Radius;
		float f = sphereCenterLS.z + m_Cascades[i].Radius;

		Matrix lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
		m_Cascades[i].ShadowViewProj = XMMatrixMultiply(lightView, lightProj);
	}

	ApplyShadowTransforms();
}

void CascadedShadowMap::SetGraphicsRootDescriptorTables(int fromSlot, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	// 5..8
	for (int i = 0; i < CASCADES_COUNT; i++)
	{
		commandList->SetGraphicsRootDescriptorTable(
			fromSlot + i,
			DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, i));
	}
}

void CascadedShadowMap::ApplyShadowTransforms()
{
	Matrix matrices[CASCADES_COUNT];
	for (size_t i = 0; i < CASCADES_COUNT; i++)
	{
		matrices[i] = m_Cascades[i].ShadowViewProj;
	}
	BaseObject::SetSMMatrices(matrices);
}

ShadowMap* CascadedShadowMap::GetShadowMap(int index)
{
	return m_Cascades[index].ShadowMapTexture;
}

Matrix CascadedShadowMap::GetShadowViewProj(int index)
{
	return m_Cascades[index].ShadowViewProj;
}



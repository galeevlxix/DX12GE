#include "ParticleSystem.h"
#include "../ShaderResources.h"
#include "../DescriptorHeaps.h"

void ParticleSystem::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	m_Texture.Load(commandList, "../../DX12GE/Resources/Particle Textures/Mike_Wazowski.png");

	vector<VertexPosition> vertices;
	float offset = 2;
	int w = 100, h = 100;

	for (size_t i = 0; i < w; i++)
	{
		for (size_t j = 0; j < h; j++)
		{
			vertices.push_back(VertexPosition(Vector3((i - w / 2.0f) * offset, 2, (j - h / 2.0f) * offset)));
		}
	}

	vector<WORD> indices;
	for (size_t i = 0; i < w * h; i++)
	{
		indices.push_back(i);
	}

	m_Mesh.OnLoad(commandList, vertices, indices);
}

void ParticleSystem::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX projMatrix, Vector3 CameraPos)
{
	commandList->SetGraphicsRootDescriptorTable(
		1,
		DescriptorHeaps::GetGPUHandle(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			m_Texture.m_SRVHeapIndex));

	ShaderResources::GetParticleCB()->ViewProjection = projMatrix;
	ShaderResources::GetParticleCB()->CameraPosition = Vector4(CameraPos.x, CameraPos.y, CameraPos.z, 1.0);
	ShaderResources::SetGraphicsParticleCB(commandList, 0);

	m_Mesh.OnRenderPointList(commandList);
}

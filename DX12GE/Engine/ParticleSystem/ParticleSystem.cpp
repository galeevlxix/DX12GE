#include "ParticleSystem.h"
#include "../ShaderResources.h"
#include "../DescriptorHeaps.h"

void ParticleSystem::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	m_Texture.Load(commandList, "../../DX12GE/Resources/Particle Textures/Mike_Wazowski.png");
	CreateParticleGroupPrototype(100);
}

void ParticleSystem::OnUpdate(float deltaTime)
{
	vector<ParticleGroup> temp;
	for (int i = 0; i < m_Particles.size(); i++)
	{
		m_Particles[i].Age += deltaTime;

		if (m_Particles[i].Age >= m_Particles[i].LifeTime)
		{
			m_Particles[i].Mesh.Release();
			m_Particles[i].Vertices.clear();
			m_Particles[i].Indices.clear();
		}
		else
		{
			temp.push_back(m_Particles[i]);
		}
	}
	m_Particles = temp;
}

void ParticleSystem::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX projMatrix, Vector3 CameraPos)
{
	commandList->SetGraphicsRootDescriptorTable(
		1,
		DescriptorHeaps::GetGPUHandle(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			m_Texture.m_SRVHeapIndex));

	ShaderResources::GetParticleCB()->ViewProjection = projMatrix;
	ShaderResources::GetParticleCB()->CameraPosition = CameraPos;

	for (int i = 0; i < m_Particles.size(); i++)
	{
		ShaderResources::GetParticleCB()->Age = m_Particles[i].Age;
		ShaderResources::SetGraphicsParticleCB(commandList, 0);
		m_Particles[i].Mesh.OnRenderPointList(commandList);
	}
}

void ParticleSystem::SpawnParticleGroup(ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, float speed, float lifeTime)
{
	ParticleGroup group = m_ParticleGroupPrototype;
	group.Position = position;
	group.LifeTime = lifeTime;

	for (size_t i = 0; i < group.Vertices.size(); i++)
	{
		group.Vertices[i].Position = group.Vertices[i].Position + position;
		group.Vertices[i].Velocity = group.Vertices[i].Velocity * speed;
	}

	group.Mesh.OnLoad(commandList, group.Vertices, group.Indices);
	m_Particles.push_back(group);
}

void ParticleSystem::CreateParticleGroupPrototype(int count)
{
	m_ParticleGroupPrototype.Position = Vector3(0, 0, 0);
	m_ParticleGroupPrototype.LifeTime = 0.0f;
	m_ParticleGroupPrototype.Age = 0.0f;

	// vertices and indices
	const float offset = 2.0f / count;
	const float increment = XM_PI * (3.0f - sqrtf(5.0f)); // Золотое сечение

	for (int i = 0; i < count; i++)
	{
		float y = ((i * offset) - 1.0f) + (offset / 2.0f);
		float r = sqrtf(1.0f - y * y); // Радиус круга на этой "широте"
		float phi = i * increment;

		float x = cosf(phi) * r;
		float z = sinf(phi) * r;

		Vector3 velocity = Vector3(x, y, z);
		velocity.Normalize();

		m_ParticleGroupPrototype.Vertices.push_back(VertexParticle(Vector3(0, 0, 0), velocity));
		m_ParticleGroupPrototype.Indices.push_back(i);
	}
}

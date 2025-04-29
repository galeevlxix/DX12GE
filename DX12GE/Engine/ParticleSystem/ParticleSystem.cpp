#include "ParticleSystem.h"
#include "../ShaderResources.h"
#include "../DescriptorHeaps.h"
#include "../Application.h"

void ParticleSystem::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	m_Texture.Load(commandList, "../../DX12GE/Resources/Particle Textures/circle_05.png");
	CreateParticleGroupPrototype(10000);

	m_Device = Application::Get().GetDevice();
}

void ParticleSystem::OnUpdate(float deltaTime, bool stop, XMMATRIX projMatrix, Vector3 CameraPos)
{
	ShaderResources::GetParticleComputeCB()->DeltaTime = !stop ? deltaTime : 0;
	ShaderResources::GetParticleComputeCB()->CameraPos = CameraPos;
	ShaderResources::GetParticleCB()->ViewProjection = projMatrix;
	ShaderResources::GetParticleCB()->CameraPosition = Vector4(CameraPos.x, CameraPos.y, CameraPos.z, 1);

	for (int i = 0; i < m_Particles.size(); i++)
	{
		if (!stop) m_Particles[i].Age += deltaTime;

		if (m_Particles[i].Age >= m_Particles[i].LifeTime)
		{
			m_Particles[i].dead = true;
		}
	}
}

void ParticleSystem::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	static bool firstRender = true;
	if (firstRender)
	{
		firstRender = false;
		return;
	}

	commandList->SetGraphicsRootDescriptorTable(
		1,
		DescriptorHeaps::GetGPUHandle(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			m_Texture.m_SRVHeapIndex));

	
	ShaderResources::SetGraphicsParticleCB(commandList, 0);

	for (int i = 0; i < m_Particles.size(); i++)
	{
		if (m_Particles[i].dead != true)
		{
			TransitionResource(commandList, m_Particles[i].Resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

			D3D12_VERTEX_BUFFER_VIEW particleVBView = {};
			particleVBView.BufferLocation = m_Particles[i].Resource->GetGPUVirtualAddress();
			particleVBView.StrideInBytes = sizeof(VertexParticle);
			particleVBView.SizeInBytes = sizeof(VertexParticle) * m_Particles[i].Vertices.size();

			commandList->IASetVertexBuffers(0, 1, &particleVBView);
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST); // если частицы как точки
			
			commandList->DrawInstanced(m_Particles[i].Vertices.size(), 1, 0, 0);

			TransitionResource(commandList, m_Particles[i].Resource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		}
	}
}

void ParticleSystem::OnComputeRender(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	static const int numOfThreads = 256;

	for (int i = 0; i < m_Particles.size(); i++)
	{
		if (m_Particles[i].dead != true)
		{
			ShaderResources::GetParticleComputeCB()->ParticleCount = m_Particles[i].Vertices.size();
			ShaderResources::GetParticleComputeCB()->Mode = 0;
			ShaderResources::SetParticleComputeCB(commandList, 0);
			commandList->SetComputeRootDescriptorTable(1, m_Particles[i].uavGPUDescHandle);
			int dispatchX = (m_Particles[i].Vertices.size() + numOfThreads - 1) / numOfThreads;
			commandList->Dispatch(dispatchX, 1, 1);
		}
	}
}

void ParticleSystem::ReadDataFromCS()
{
	
}

void ParticleSystem::SpawnParticleGroup(ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, float speed, float lifeTime)
{
	ParticleGroup group = m_ParticleGroupPrototype;
	group.Position = position;
	group.LifeTime = lifeTime;

	for (size_t i = 0; i < group.Vertices.size(); i++)
	{
		group.Vertices[i].Position = group.Vertices[i].Position + position;
		group.Vertices[i].Speed = group.Vertices[i].Speed * speed;
	}

	// CREATE BUFFER

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = sizeof(VertexParticle) * group.Vertices.size();
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // обязательно для UAV!

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(
		m_Device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&group.Resource)));

	// LOAD DATA
	
	D3D12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(VertexParticle) * group.Vertices.size());

	CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(m_Device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadBuffer)));

	void* mappedData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_UploadBuffer->Map(0, &readRange, &mappedData));
	memcpy(mappedData, group.Vertices.data(), sizeof(VertexParticle) * group.Vertices.size());
	m_UploadBuffer->Unmap(0, nullptr);

	TransitionResource(commandList, group.Resource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	commandList->CopyBufferRegion(
		group.Resource.Get(), 
		0,           
		m_UploadBuffer.Get(),
		0,              
		sizeof(VertexParticle) * group.Vertices.size());

	TransitionResource(commandList, group.Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// CREATE UAV

	static int indexInHeap = 2;
	group.uavCPUDescHandle = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, indexInHeap);
	group.uavGPUDescHandle = DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, indexInHeap);
	indexInHeap++;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = group.Vertices.size();
	uavDesc.Buffer.StructureByteStride = sizeof(VertexParticle);
	uavDesc.Format = group.Resource->GetDesc().Format;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	m_Device->CreateUnorderedAccessView(group.Resource.Get(), nullptr, &uavDesc, group.uavCPUDescHandle);
	m_Particles.push_back(group);
}

void ParticleSystem::CreateParticleGroupPrototype(int count)
{
	m_ParticleGroupPrototype.Position = Vector3(0, 0, 0);
	m_ParticleGroupPrototype.LifeTime = 0.0f;
	m_ParticleGroupPrototype.Age = 0.0f;
	m_ParticleGroupPrototype.dead = false;

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

		m_ParticleGroupPrototype.Vertices.push_back(VertexParticle(velocity * 8, Vector3(0, 0, 0)));
		m_ParticleGroupPrototype.Indices.push_back(i);
	}
}

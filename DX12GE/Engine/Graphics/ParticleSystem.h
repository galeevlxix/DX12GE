#pragma once
#include "../Base/DX12LibPCH.h"
#include "../Base/SimpleMath.h"
#include "Texture.h"
#include "Object3D.h"
#include "VertexStructures.h"

#include <vector>

using namespace DirectX::SimpleMath;
using namespace std;

struct ParticleGroup
{
	vector<VertexParticle> Vertices;
	vector<WORD> Indices;
	
	Vector3 Position;
	float Age;
	float LifeTime;
	bool dead;

	ComPtr<ID3D12Resource> Resource;

	CD3DX12_CPU_DESCRIPTOR_HANDLE uavCPUDescHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE uavGPUDescHandle;
};

class ParticleSystem
{
public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnUpdate(float deltaTime, bool stop, XMMATRIX viewProjMatrix, Vector3 CameraPos);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnUpdateComputeRender(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnSortComputeRender(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void SpawnParticleGroup(ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, float speed, float lifeTime);
private:
	Texture m_Texture;

	ParticleGroup m_ParticleGroupPrototype;
	vector<ParticleGroup> m_Particles;
	
	ComPtr<ID3D12Device2> m_Device;
	ComPtr<ID3D12Resource> m_UploadBuffer;
	
	bool stopUpdate = false;
	bool stopSort = false;

	void CreateParticleGroupPrototype(int count);
};
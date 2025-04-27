#pragma once
#include "../DX12LibPCH.h"
#include "../Texture.h"
#include "../../Game/BaseObject.h"
#include "../SimpleMath.h"
#include <vector>
#include "../VertexStructures.h"

using namespace DirectX::SimpleMath;
using namespace std;

struct ParticleGroup
{
	vector<VertexParticle> Vertices;
	vector<WORD> Indices;
	BaseObject Mesh;
	Vector3 Position;
	float Age;
	float LifeTime;
	bool dead;

	ComPtr<ID3D12Resource> Resource;
	//ComPtr<ID3D12Resource> ReadbackBuffer;

	CD3DX12_CPU_DESCRIPTOR_HANDLE uavCPUDescHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE uavGPUDescHandle;
};

class ParticleSystem
{
public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnUpdate(float deltaTime, bool stop);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix, Vector3 CameraPos);
	void OnComputeRender(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void ReadDataFromCS();

	void SpawnParticleGroup(ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, float speed, float lifeTime);
private:
	Texture m_Texture;

	ParticleGroup m_ParticleGroupPrototype;
	vector<ParticleGroup> m_Particles;
	
	ComPtr<ID3D12Device2> m_Device;
	ComPtr<ID3D12Resource> m_UploadBuffer;
	

	void CreateParticleGroupPrototype(int count);
};
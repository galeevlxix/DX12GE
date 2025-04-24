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
};

class ParticleSystem
{
public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnUpdate(float deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix, Vector3 CameraPos);

	void SpawnParticleGroup(ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, float speed, float lifeTime);
private:
	Texture m_Texture;

	ParticleGroup m_ParticleGroupPrototype;
	vector<ParticleGroup> m_Particles;

	void CreateParticleGroupPrototype(int count);
};
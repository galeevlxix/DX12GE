#pragma once
#include "../DX12LibPCH.h"
#include "../Texture.h"
#include "../../Game/BaseObject.h"
#include "../SimpleMath.h"
#include <vector>
#include "../VertexStructures.h"

using namespace DirectX::SimpleMath;
using namespace std;

class ParticleSystem
{
public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix, Vector3 CameraPos);

private:
	Texture m_Texture;
	BaseObject m_Mesh;
};
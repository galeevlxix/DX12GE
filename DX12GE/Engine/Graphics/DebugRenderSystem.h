#pragma once

#include "Mesh3DComponent.h"
#include "../Base/SimpleMath.h"
#include "../Pipelines/SimplePipeline.h"
#include "VertexStructures.h"
#include <vector>

using namespace std;
using namespace DirectX;

class DebugRenderSystem
{
private:
	
	Mesh3DComponent m_Lines;
	Mesh3DComponent m_Quads;
	Mesh3DComponent m_Meshes;

	vector<VertexPositionColor> linesVertices;
	vector<WORD> linesIndices;
	bool isLinesDirty = true;

	void CreateLine(const Vector3& pos0, const Vector3& pos1, const Color& color);
public:
	bool canDraw = true;

	void Update(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void Draw(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
	void Clear();

	void DrawLine(const Vector3& pos0, const Vector3& pos1, const Color& color);
	void DrawBoundingBox(const BoundingBox& box);
	void DrawBoundingBox(const BoundingBox& box, const Matrix& transform);
	void DrawArrow(const Vector3& p0, const Vector3& p1, const Color& color, const Vector3& n);
	void DrawPoint(const Vector3& pos, const float& size);
	void DrawCircle(const double& radius, const Color& color, const Matrix& transform, int density);
	void DrawSphere(const double& radius, const Color& color, const Matrix& transform, int density);
	void DrawPlane(const Vector4& p, const Color& color, float sizeWidth, float sizeNormal, bool drawCenterCross);
	void DrawFrustrum(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

	void Destroy();
};
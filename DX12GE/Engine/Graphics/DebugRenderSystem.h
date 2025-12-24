#pragma once

#include "Mesh3DComponent.h"
#include "../Base/SimpleMath.h"
#include "../Base/CollisionBox.h"
#include "../Pipelines/SimplePipeline.h"
#include "VertexStructures.h"
#include <vector>

using namespace std;
using namespace DirectX;

class DebugRenderSystem
{
private:
	
	Mesh3DComponent m_Lines;

	vector<VertexPositionColor> linesVertices;
	vector<WORD> linesIndices;

	bool isLinesDirty = true;

	void CreateLine(const Vector3& pos0, const Vector3& pos1, const Color& color);
public:
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);

	void DrawLine(const Vector3& pos0, const Vector3& pos1, const Color& color);
	void DrawBoundingBox(const BoundingBox& box);
	void DrawBoundingBox(const BoundingBox& box, const Matrix& transform);
	void DrawBoundingBox(const CollisionBox& box, const Matrix& transform);
	void DrawArrow(const Vector3& p0, const Vector3& p1, const Color& color, const Vector3& n);
	void DrawArrow(const Vector3& p0, const Vector3& p1, const Vector3& color, const Vector3& n);
	void DrawPoint(const Vector3& pos, const float& size);
	void DrawPoint(const Vector3& pos, const float& size, const Color& color);
	void DrawPoint(const Vector3& pos, const float& size, const Vector3& color);
	void DrawCircle(const double& radius, const Color& color, const Matrix& transform, int density);
	void DrawSphere(const double& radius, const Color& color, const Matrix& transform, int density);
	void DrawSphere(const double& radius, const Vector3& color, const Matrix& transform, int density);
	void DrawPlane(const Vector4& p, const Color& color, float sizeWidth, float sizeNormal, bool drawCenterCross);
	void DrawFrustrum(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

	void Clear();
	void Destroy();
};
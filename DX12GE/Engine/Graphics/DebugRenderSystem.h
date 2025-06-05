#pragma once

#include "../Base/SimpleMath.h"
#include "../Pipelines/SimplePipeline.h"
#include "VertexStructures.h"
#include "Camera.h"
#include <vector>

using namespace std;
using namespace DirectX;

class DebugRenderSystem
{
private:
	
	Mesh3D m_Lines;
	Mesh3D m_Quads;
	Mesh3D m_Meshes;

	vector<VertexPositionColor> linesVertices;
	vector<WORD> linesIndices;
	bool isLinesDirty = true;
	
	Camera* m_Camera;

	void CreateLine(const Vector3& pos0, const Vector3& pos1, const Color& color);
public:
	bool canDraw = true;

	void Initialize(Camera* camera, ComPtr<ID3D12Device2> device);

	void Update(ComPtr<ID3D12GraphicsCommandList2> commandList);
	void Draw(ComPtr<ID3D12GraphicsCommandList2> commandList);
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
};
#pragma once

#include "SimplePipeline.h"
//#include "TexturedPipeline.h"
#include "../SimpleMath.h"
#include "../../Game/Camera.h"
#include <vector>

using namespace std;
using namespace DirectX;

class DebugRenderSystem
{
private:
	SimplePipeline m_SimplePipeline;
	//TexturedPipeline m_TexturedPipeline;

	BaseObject m_Lines;
	BaseObject m_Quads;
	BaseObject m_Meshes;

	vector<VertexPositionColor> linesVertices;
	vector<WORD> linesIndices;
	bool isLinesDirty = true;

	Camera* m_Camera;

	void CreateLine(const Vector3& pos0, const Vector3& pos1, const Color& color);
public:

	void Init(Camera* camera, ComPtr<ID3D12Device2> device);

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
};
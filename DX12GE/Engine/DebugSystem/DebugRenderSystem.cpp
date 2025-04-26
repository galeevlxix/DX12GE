#include "DebugRenderSystem.h"
#include "../Application.h"

void DebugRenderSystem::Initialize(Camera* camera, ComPtr<ID3D12Device2> device)
{
	m_SimplePipeline.Initialize(device);
	m_Camera = camera;
}

void DebugRenderSystem::Update(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	if (isLinesDirty)
	{
		m_Lines.OnLoad<VertexPositionColor>(commandList, linesVertices, linesIndices);
		isLinesDirty = false;
	}
	m_Lines.OnUpdate();
}

void DebugRenderSystem::Draw(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	if (m_Camera == nullptr || isLinesDirty || !canDraw) return;
	m_SimplePipeline.Set(commandList);
	BaseObject::SetShadowPass(true);
	m_Lines.OnRenderLineList(commandList, m_Camera->GetViewProjMatrix());
	BaseObject::SetShadowPass(false);
}

void DebugRenderSystem::Clear()
{
	linesVertices.clear();
	linesIndices.clear();
	m_Lines.Release();
}

void DebugRenderSystem::CreateLine(const Vector3& pos0, const Vector3& pos1, const Color& color)
{
	VertexPositionColor v1({ XMFLOAT3(pos0.x, pos0.y, pos0.z), XMFLOAT3(color.x, color.y, color.z) });
	VertexPositionColor v2({ XMFLOAT3(pos1.x, pos1.y, pos1.z), XMFLOAT3(color.x, color.y, color.z) });

	linesVertices.push_back(v1);
	linesVertices.push_back(v2);

	linesIndices.push_back(static_cast<WORD>(linesVertices.size() - 2));
	linesIndices.push_back(static_cast<WORD>(linesVertices.size() - 1));

	isLinesDirty = true;
}

void DebugRenderSystem::DrawLine(const Vector3& pos0, const Vector3& pos1, const Color& color)
{
	CreateLine(pos0, pos1, color);
}

void DebugRenderSystem::DrawBoundingBox(const BoundingBox& box)
{
	Vector3 corners[8];

	box.GetCorners(&corners[0]);

	CreateLine(corners[0], corners[1], Color(0.0f, 1.0f, 0.0f, 1.0f));
	CreateLine(corners[1], corners[2], Color(0.0f, 1.0f, 0.0f, 1.0f));
	CreateLine(corners[2], corners[3], Color(0.0f, 1.0f, 0.0f, 1.0f));
	CreateLine(corners[3], corners[0], Color(0.0f, 1.0f, 0.0f, 1.0f));

	CreateLine(corners[4], corners[5], Color(0.0f, 1.0f, 1.0f, 1.0f));
	CreateLine(corners[5], corners[6], Color(0.0f, 1.0f, 1.0f, 1.0f));
	CreateLine(corners[6], corners[7], Color(0.0f, 1.0f, 1.0f, 1.0f));
	CreateLine(corners[7], corners[4], Color(0.0f, 1.0f, 1.0f, 1.0f));

	CreateLine(corners[0], corners[4], Color(0.0f, 0.0f, 1.0f, 1.0f));
	CreateLine(corners[1], corners[5], Color(0.0f, 0.0f, 1.0f, 1.0f));
	CreateLine(corners[2], corners[6], Color(0.0f, 0.0f, 1.0f, 1.0f));
	CreateLine(corners[3], corners[7], Color(0.0f, 0.0f, 1.0f, 1.0f));
}

void DebugRenderSystem::DrawBoundingBox(const BoundingBox& box, const Matrix& transform)
{
	Vector3 corners[8];
	box.GetCorners(&corners[0]);

	for (auto& corner : corners) {
		corner = Vector3::Transform(corner, transform);
	}

	CreateLine(corners[0], corners[1], Color(0.0f, 1.0f, 0.0f, 1.0f));
	CreateLine(corners[1], corners[2], Color(0.0f, 1.0f, 0.0f, 1.0f));
	CreateLine(corners[2], corners[3], Color(0.0f, 1.0f, 0.0f, 1.0f));
	CreateLine(corners[3], corners[0], Color(0.0f, 1.0f, 0.0f, 1.0f));

	CreateLine(corners[4], corners[5], Color(0.0f, 1.0f, 1.0f, 1.0f));
	CreateLine(corners[5], corners[6], Color(0.0f, 1.0f, 1.0f, 1.0f));
	CreateLine(corners[6], corners[7], Color(0.0f, 1.0f, 1.0f, 1.0f));
	CreateLine(corners[7], corners[4], Color(0.0f, 1.0f, 1.0f, 1.0f));

	CreateLine(corners[0], corners[4], Color(0.0f, 0.0f, 1.0f, 1.0f));
	CreateLine(corners[1], corners[5], Color(0.0f, 0.0f, 1.0f, 1.0f));
	CreateLine(corners[2], corners[6], Color(0.0f, 0.0f, 1.0f, 1.0f));
	CreateLine(corners[3], corners[7], Color(0.0f, 0.0f, 1.0f, 1.0f));
}

void DebugRenderSystem::DrawArrow(const Vector3& p0, const Vector3& p1, const Color& color, const Vector3& n)
{
	CreateLine(p0, p1, color);

	auto a = Vector3::Lerp(p0, p1, 0.85f);

	auto diff = p1 - p0;
	auto side = n.Cross(diff) * 0.05f;

	CreateLine(a + side, p1, color);
	CreateLine(a - side, p1, color);
}

void DebugRenderSystem::DrawPoint(const Vector3& pos, const float& size)
{
	Vector3 red(1.0f, 0.0f, 0.0f);
	Vector3 green(0.0f, 1.0f, 0.0f);
	Vector3 blue(0.0f, 0.0f, 1.0f);

	linesVertices.emplace_back(VertexPositionColor
		{
			Vector3(pos.x + size, pos.y, pos.z),
			red
		});
	linesIndices.emplace_back(linesVertices.size() - 1);
	linesVertices.emplace_back(VertexPositionColor
		{
			Vector3(pos.x - size, pos.y, pos.z),
			red
		});
	linesIndices.emplace_back(linesVertices.size() - 1);
	linesVertices.emplace_back(VertexPositionColor
		{
			Vector3(pos.x, pos.y + size, pos.z),
			green
		});
	linesIndices.emplace_back(linesVertices.size() - 1);
	linesVertices.emplace_back(VertexPositionColor
		{
			Vector3(pos.x, pos.y - size, pos.z),
			green
		});
	linesIndices.emplace_back(linesVertices.size() - 1);
	linesVertices.emplace_back(VertexPositionColor
		{
			Vector3(pos.x, pos.y, pos.z + size),
			blue
		});
	linesIndices.emplace_back(linesVertices.size() - 1);
	linesVertices.emplace_back(VertexPositionColor
		{
			Vector3(pos.x, pos.y, pos.z - size),
			blue
		});
	linesIndices.emplace_back(linesVertices.size() - 1);

	isLinesDirty = true;
}

void DebugRenderSystem::DrawCircle(const double& radius, const Color& color, const Matrix& transform, int density)
{
	double angleStep = DirectX::XM_PI * 2 / density;

	for (int i = 0; i < density; i++)
	{
		auto point0X = radius * cos(angleStep * i);
		auto point0Y = radius * sin(angleStep * i);

		auto point1X = radius * cos(angleStep * (i + 1));
		auto point1Y = radius * sin(angleStep * (i + 1));

		auto p0 = Vector3::Transform(Vector3(static_cast<float>(point0X), static_cast<float>(point0Y), 0), transform);
		auto p1 = Vector3::Transform(Vector3(static_cast<float>(point1X), static_cast<float>(point1Y), 0), transform);

		CreateLine(p0, p1, color);
	}
}

void DebugRenderSystem::DrawSphere(const double& radius, const Color& color, const Matrix& transform, int density)
{
	DrawCircle(radius, color, transform, density);
	DrawCircle(radius, color, Matrix::CreateRotationX(DirectX::XM_PIDIV2) * transform, density);
	DrawCircle(radius, color, Matrix::CreateRotationY(DirectX::XM_PIDIV2) * transform, density);
}

void DebugRenderSystem::DrawPlane(const Vector4& p, const Color& color, float sizeWidth, float sizeNormal, bool drawCenterCross)
{
	auto dir = Vector3(p.x, p.y, p.z);
	if (dir.Length() == 0.0f) return;
	dir.Normalize();

	auto up = Vector3(0, 0, 1);
	auto right = dir.Cross(up);
	if (right.Length() < 0.01f) {
		up = Vector3(0, 1, 0);
		right = dir.Cross(up);
	}
	right.Normalize();

	up = right.Cross(dir);

	auto pos = -dir * p.w;

	auto leftPoint = pos - right * sizeWidth;
	auto rightPoint = pos + right * sizeWidth;
	auto downPoint = pos - up * sizeWidth;
	auto upPoint = pos + up * sizeWidth;

	CreateLine(leftPoint + up * sizeWidth, rightPoint + up * sizeWidth, color);
	CreateLine(leftPoint - up * sizeWidth, rightPoint - up * sizeWidth, color);
	CreateLine(downPoint - right * sizeWidth, upPoint - right * sizeWidth, color);
	CreateLine(downPoint + right * sizeWidth, upPoint + right * sizeWidth, color);


	if (drawCenterCross) {
		CreateLine(leftPoint, rightPoint, color);
		CreateLine(downPoint, upPoint, color);
	}

	DrawPoint(pos, 0.5f);
	DrawArrow(pos, pos + dir * sizeNormal, color, right);
}

Vector3 ToVec3(Vector4 vec)
{
	return Vector3(vec.x, vec.y, vec.z);
}

vector<Vector4> GetFrustumCornersWorldSpace(const Matrix& view, const Matrix& proj)
{
	const auto viewProj = view * proj;
	const auto inv = viewProj.Invert();

	vector<Vector4> frustumCorners;
	frustumCorners.reserve(8);
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const Vector4 pt = Vector4::Transform(Vector4(2.0f * x - 1.0f, 2.0f * y - 1.0f, z, 1.0f), inv);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}

void DebugRenderSystem::DrawFrustrum(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj)
{
	const vector<Vector4> corners = GetFrustumCornersWorldSpace(view, proj);

	auto invView = view.Invert();
	DrawPoint(invView.Translation(), 1.0f);

	DrawLine(ToVec3(corners[0]), ToVec3(corners[1]), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(ToVec3(corners[2]), ToVec3(corners[3]), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(ToVec3(corners[4]), ToVec3(corners[5]), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	DrawLine(ToVec3(corners[6]), ToVec3(corners[7]), Vector4(0.0f, 0.0f, 1.0f, 1.0f));

	DrawLine(ToVec3(corners[0]), ToVec3(corners[2]), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(ToVec3(corners[1]), ToVec3(corners[3]), Vector4(0.0f, 0.5f, 0.0f, 1.0f));
	DrawLine(ToVec3(corners[4]), ToVec3(corners[6]), Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	DrawLine(ToVec3(corners[5]), ToVec3(corners[7]), Vector4(0.0f, 0.5f, 0.0f, 1.0f));

	DrawLine(ToVec3(corners[0]), ToVec3(corners[4]), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	DrawLine(ToVec3(corners[1]), ToVec3(corners[5]), Vector4(0.5f, 0.0f, 0.0f, 1.0f));
	DrawLine(ToVec3(corners[2]), ToVec3(corners[6]), Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	DrawLine(ToVec3(corners[3]), ToVec3(corners[7]), Vector4(0.5f, 0.0f, 0.0f, 1.0f));
}

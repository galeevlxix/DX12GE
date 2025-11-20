#include "../DebugRenderSystem.h"
#include "../../Base/Application.h"

void DebugRenderSystem::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
	if (linesVertices.size() == 0) return;

	if (isLinesDirty)
	{
		m_Lines.OnLoad<VertexPositionColor>(commandList, linesVertices, linesIndices, true);
		isLinesDirty = false;
	}

	XMMATRIX mvpMatrix = XMMatrixMultiply(Matrix::Identity, viewProjMatrix);
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

	m_Lines.OnRender(commandList, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
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

void DebugRenderSystem::DrawTriangle(const Vector3& pos0, const Vector3& pos1, const Vector3& pos2, const Color& color)
{
	CreateLine(pos0, pos1, color);
	CreateLine(pos1, pos2, color);
	CreateLine(pos2, pos0, color);
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

void DebugRenderSystem::DrawArrow(const Vector3& p0, const Vector3& p1, const Vector3& color, const Vector3& n)
{
	DrawArrow(p0, p1, Color(color.x, color.y, color.z), n);
}

void DebugRenderSystem::DrawPoint(const Vector3& pos, const float& size)
{
	CreateLine(Vector3(pos.x + size, pos.y, pos.z), Vector3(pos.x - size, pos.y, pos.z), Color(1.0f, 0.0f, 0.0f));
	CreateLine(Vector3(pos.x, pos.y + size, pos.z), Vector3(pos.x, pos.y - size, pos.z), Color(0.0f, 1.0f, 0.0f));
	CreateLine(Vector3(pos.x, pos.y, pos.z + size), Vector3(pos.x, pos.y, pos.z - size), Color(0.0f, 0.0f, 1.0f));
}

void DebugRenderSystem::DrawPoint(const Vector3& pos, const float& size, const Color& color)
{
	CreateLine(Vector3(pos.x + size, pos.y, pos.z), Vector3(pos.x - size, pos.y, pos.z), color);
	CreateLine(Vector3(pos.x, pos.y + size, pos.z), Vector3(pos.x, pos.y - size, pos.z), color);
	CreateLine(Vector3(pos.x, pos.y, pos.z + size), Vector3(pos.x, pos.y, pos.z - size), color);
}

void DebugRenderSystem::DrawPoint(const Vector3& pos, const float& size, const Vector3& color)
{
	DrawPoint(pos, size, Color(color.x, color.y, color.z));
}

void DebugRenderSystem::DrawPoint(const Vector3& pos, const float& size, const Color& color)
{
	CreateLine(Vector3(pos.x + size, pos.y, pos.z), Vector3(pos.x - size, pos.y, pos.z), color);
	CreateLine(Vector3(pos.x, pos.y + size, pos.z), Vector3(pos.x, pos.y - size, pos.z), color);
	CreateLine(Vector3(pos.x, pos.y, pos.z + size), Vector3(pos.x, pos.y, pos.z - size), color);
}

void DebugRenderSystem::DrawPoint(const Vector3& pos, const float& size, const Vector3& color)
{
	DrawPoint(pos, size, Color(color.x, color.y, color.z));
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

void DebugRenderSystem::DrawSphere(const double& radius, const Vector3& color, const Matrix& transform, int density)
{
	DrawSphere(radius, Color(color.x, color.y, color.z), transform, density);
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
	for (float x = 0.0f; x < 2; ++x)
	{
		for (float y = 0.0f; y < 2; ++y)
		{
			for (float z = 0.0f; z < 2; ++z)
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

static void BuildBasisFromForward(const Vector3& forward, Vector3& R, Vector3& U)
{
	Vector3 worldUp = (std::abs(forward.y) < 0.99f) ? Vector3{ 0, 1, 0 } : Vector3{ 1, 0, 0 };

	R = worldUp.Cross(forward);
	R.Normalize();

	U = forward.Cross(R);
	U.Normalize();
}

void DebugRenderSystem::DrawCone(const Vector3& pos, const Vector3& forward, float angle, const Color& color)
{
	float length = 1.0f;
	int segments = 8;

	if (forward.Length() < 0.0001f)
	{
		DrawSphere(0.2f, Color(1.0f, 1.0f, 1.0f), Matrix::CreateTranslation(pos), 10);
		return;
	}

	if (angle <= 0.0001f)
	{
		CreateLine(pos, pos + forward * length, color);
		return;
	}

	Vector3 R, U;
	BuildBasisFromForward(forward, R, U);

	float half = 0.5f * angle;
	float c = std::cos(half);
	float s = std::sin(half);

	std::vector<Vector3> ring;
	ring.reserve(segments);

	for (int i = 0; i < segments; ++i)
	{
		float t = (2.0f * PI) * (float)i / (float)segments;
		float ct = std::cos(t);
		float st = std::sin(t);

		Vector3 around = (R * ct) + (U * st); 
		Vector3 dir = (forward * c) + (around * s);

		Vector3 p = pos + dir * length;
		ring.push_back(p);

		CreateLine(pos, p, color);
	}

	for (int i = 0; i < segments; ++i)
	{
		const Vector3& a = ring[i];
		const Vector3& b = ring[(i + 1) % segments];
		CreateLine(a, b, color);
	}
}

void DebugRenderSystem::DrawBoundingBox(const CollisionBox& box, const Matrix& transform)
{
	Vector3 center((box.x_max + box.x_min) / 2.0f, (box.y_max + box.y_min) / 2.0f, (box.z_max + box.z_min) / 2.0f);
	Vector3 extents((box.x_max - box.x_min) / 2.0f, (box.y_max - box.y_min) / 2.0f, (box.z_max - box.z_min) / 2.0f);
	BoundingBox bounding(center, extents);
	DrawBoundingBox(bounding, transform);
}

void DebugRenderSystem::DrawCellularFieldAndAxes(const Vector3& cameraPos)
{
	const float camPosX = roundf(cameraPos.x);
	const float camPosY = roundf(abs(cameraPos.y));
	const float camPosZ = roundf(cameraPos.z);

	DrawLine(Vector3(-CellularFieldFieldSizeLevel1 + cameraPos.x, 0, 0), Vector3(CellularFieldFieldSizeLevel1 + cameraPos.x, 0, 0), AxisXColor);
	DrawLine(Vector3(0, -CellularFieldFieldSizeLevel1 + cameraPos.y, 0), Vector3(0, CellularFieldFieldSizeLevel1 + cameraPos.y, 0), AxisYColor);
	DrawLine(Vector3(0, 0, -CellularFieldFieldSizeLevel1 + cameraPos.z), Vector3(0, 0, CellularFieldFieldSizeLevel1 + cameraPos.z), AxisZColor);

	for (int x = -CellularFieldFieldSizeLevel1 + static_cast<int>(camPosX); x <= CellularFieldFieldSizeLevel1 + static_cast<int>(camPosX); x++)
	{
		if (x == 0) continue;

		float xFloat = static_cast<float>(x);

		if (x % 64 == 0)
		{
			DrawLine(Vector3(xFloat, 0, -CellularFieldFieldSizeLevel1 + camPosZ), Vector3(xFloat, 0, CellularFieldFieldSizeLevel1 + camPosZ), CellularFieldColorLevel1);
		}
		else if (x % 8 == 0 && camPosY <= CellularFieldHeightLevel2 && abs(xFloat - camPosX) <= CellularFieldFieldSizeLevel2)
		{
			DrawLine(Vector3(xFloat, 0, -CellularFieldFieldSizeLevel2 + camPosZ), Vector3(xFloat, 0, CellularFieldFieldSizeLevel2 + camPosZ), CellularFieldColorLevel2);
		}
		else if (camPosY <= CellularFieldHeightLevel3 && abs(xFloat - camPosX) <= CellularFieldFieldSizeLevel3)
		{
			DrawLine(Vector3(xFloat, 0, -CellularFieldFieldSizeLevel3 + camPosZ), Vector3(xFloat, 0, CellularFieldFieldSizeLevel3 + camPosZ), CellularFieldColorLevel3);
		}
	}

	for (int z = -CellularFieldFieldSizeLevel1 + static_cast<int>(camPosZ); z <= CellularFieldFieldSizeLevel1 + static_cast<int>(camPosZ); z++)
	{
		if (z == 0) continue;

		float zFloat = static_cast<float>(z);

		if (z % 64 == 0)
		{
			DrawLine(Vector3(-CellularFieldFieldSizeLevel1 + camPosX, 0, zFloat), Vector3(CellularFieldFieldSizeLevel1 + camPosX, 0, zFloat), CellularFieldColorLevel1);
		}
		else if (z % 8 == 0 && camPosY <= CellularFieldHeightLevel2 && abs(zFloat - camPosZ) <= CellularFieldFieldSizeLevel2)
		{
			DrawLine(Vector3(-CellularFieldFieldSizeLevel2 + camPosX, 0, zFloat), Vector3(CellularFieldFieldSizeLevel2 + camPosX, 0, zFloat), CellularFieldColorLevel2);
		}
		else if (camPosY <= CellularFieldHeightLevel3 && abs(zFloat - camPosZ) <= CellularFieldFieldSizeLevel3)
		{
			DrawLine(Vector3(-CellularFieldFieldSizeLevel3 + camPosX, 0, zFloat), Vector3(CellularFieldFieldSizeLevel3 + camPosX, 0, zFloat), CellularFieldColorLevel3);
		}
	}
}

void DebugRenderSystem::Clear()
{
	linesVertices.clear();
	linesIndices.clear();
}

void DebugRenderSystem::Destroy()
{
	Clear();
	m_Lines.Destroy();
}

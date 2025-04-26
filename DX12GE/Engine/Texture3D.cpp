#include "Texture3D.h"
#include "Application.h"
#include "PerlinNoise.h"
#include <vector>
#include "SimpleMath.h"

using namespace std;
using namespace DirectX::SimpleMath;

void Texture3D::Load(ComPtr<ID3D12GraphicsCommandList2> commandList, int width, int height, int depth)
{
	auto device = Application::Get().GetDevice();

	vector<Vector3> data;
	data.resize(width * height * depth);

	PerlinNoise randomGenerator;

	for (float z = 0; z < depth; z++)
	{
		for (float y = 0; y < height; ++y)
		{
			for (float x = 0; x < width; ++x)
			{
				Vector3 noiseVector;
				noiseVector.x = randomGenerator.Perlin3D(x / width, y / height, z / depth);
				noiseVector.y = randomGenerator.Perlin3D(y / height, z / depth, x / width);
				noiseVector.z = randomGenerator.Perlin3D(z / depth, x / width, y / height);
				noiseVector.Normalize();
				data[x + y * width + z * width * height] = noiseVector;
			}
		}
	}

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = depth;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

}

void Texture3D::CreateShaderResourceView(D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc)
{
}

void Texture3D::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture3D::GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return D3D12_CPU_DESCRIPTOR_HANDLE();
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture3D::GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return D3D12_GPU_DESCRIPTOR_HANDLE();
}

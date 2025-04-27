#include "Texture3D.h"
#include "Application.h"
#include "PerlinNoise.h"
#include <vector>
#include "SimpleMath.h"
#include <cstdlib> // для srand и rand
#include <ctime>   // для time
#include "DescriptorHeaps.h"
#include "ShaderResources.h"

using namespace std;
using namespace DirectX::SimpleMath;

// Генерация float в диапазоне [-1, 1]
float RandomFloat() 
{
	return 2.0f * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) - 1.0f;
}

float GetFloatValue(int pos, static int border, float value)
{
	static const float maxForceValue = 5;

	if (pos == 0) 
		return maxForceValue;
	if (pos == border - 1) 
		return -maxForceValue;
	return value;
}

Vector3 GetVectorValue(int x, int y, int z, int width, int height, int depth)
{
	Vector3 force = Vector3(RandomFloat(), RandomFloat(), RandomFloat());
	force.Normalize();

	force.x = GetFloatValue(x, width, force.x);
	force.y = GetFloatValue(y, height, force.y);
	force.z = GetFloatValue(z, depth, force.z);

	return force;
}

void Texture3D::Load(ComPtr<ID3D12GraphicsCommandList2> commandList, int width, int height, int depth)
{
	auto device = Application::Get().GetDevice();

	// CREATE DATA

	vector<XMFLOAT4> data;
	data.resize(width * height * depth);
	
	srand(static_cast<unsigned int>(time(0)));

	for (float z = 0; z < depth; z++)
	{
		for (float y = 0; y < height; ++y)
		{
			for (float x = 0; x < width; ++x)
			{
				int index = x + y * width + z * width * height;
				Vector3 p = GetVectorValue(x, y, z, width, height, depth);
				data[index] = XMFLOAT4(p.x, p.y, p.z, 0.0f);
			}
		}
	}

	// CREATE RESOURCE

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = depth;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };

	ThrowIfFailed(
		device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_Resource)));

	// LOAD DATA

	UINT64 uploadBufferSize = 0;
	device->GetCopyableFootprints(&texDesc, 0, 1, 0, nullptr, nullptr, nullptr, &uploadBufferSize);

	CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	ThrowIfFailed(device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_UploadBuffer)));

	D3D12_SUBRESOURCE_DATA texData = {};
	texData.pData = reinterpret_cast<const void*>(data.data());
	texData.RowPitch = width * sizeof(XMFLOAT4);
	texData.SlicePitch = texData.RowPitch * height;

	UpdateSubresources(commandList.Get(), m_Resource.Get(), m_UploadBuffer.Get(), 0, 0, 1, &texData);

	TransitionResource(commandList, m_Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// CREATE SRV

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture3D.MipLevels = 1;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;

	m_SRVHeapIndex = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
		DescriptorHeaps::GetCPUHandle(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			m_SRVHeapIndex)
	);

	device->CreateShaderResourceView(m_Resource.Get(), &srvDesc, handle);
}

void Texture3D::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	commandList->SetComputeRootDescriptorTable(
		2,
		DescriptorHeaps::GetGPUHandle(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			m_SRVHeapIndex)
	);
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture3D::GetCpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return DescriptorHeaps::GetCPUHandle(heapType, m_SRVHeapIndex);
}

D3D12_GPU_DESCRIPTOR_HANDLE Texture3D::GetGpuDescHandle(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return DescriptorHeaps::GetGPUHandle(heapType, m_SRVHeapIndex);
}

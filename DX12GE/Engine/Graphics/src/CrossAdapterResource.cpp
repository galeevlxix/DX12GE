#include "../CrossAdapterResource.h"

CrossAdapterResource::CrossAdapterResource(
	ComPtr<ID3D12Device2> primaryDevice, 
	ComPtr<ID3D12Device2> secondDevice, 
	UINT width, UINT height, DXGI_FORMAT format)
{
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; 
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;

    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);



}

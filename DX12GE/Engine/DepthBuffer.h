#include "DX12LibPCH.h"

class DepthBuffer
{
private:
    // Depth buffer.
    ComPtr<ID3D12Resource> DepthBuffer;
public:
    // Descriptor heap for depth buffer.
    ComPtr<ID3D12DescriptorHeap> DSVHeap;

    void InitDSV();
    void ResizeDepthBuffer(int width, int height);
    void ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, FLOAT depth = 1.0f);
};
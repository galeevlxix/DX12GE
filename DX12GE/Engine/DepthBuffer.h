#include "DX12LibPCH.h"

class DepthBuffer
{
public:
    // Depth buffer.
    ComPtr<ID3D12Resource> m_DepthBuffer;
    // Descriptor heap for depth buffer.
    ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

    void InitDSV();
    void ResizeDepthBuffer(int width, int height);
};
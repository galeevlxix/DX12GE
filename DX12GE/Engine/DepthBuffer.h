#include "DX12LibPCH.h"
#include "Texture.h"

class DepthBuffer
{
private:
    // Depth buffer.
    //ComPtr<ID3D12Resource> DepthBuffer;
    Texture DepthBufferTexture;
    UINT dsvCpuHandleIndex = 1;
public:
    // Descriptor heap for depth buffer.
    void ResizeDepthBuffer(int width, int height);
    void ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, FLOAT depth = 1.0f);
};
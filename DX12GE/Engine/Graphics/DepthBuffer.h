#include "../Base/DX12LibPCH.h"
#include "TextureComponent.h"

class DepthBuffer
{
private:
    // Depth buffer.
    TextureComponent DepthBufferTexture;
public:
    UINT dsvCpuHandleIndex = -1;

    void Init();
    void ResizeDepthBuffer(int width, int height);
    void ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, FLOAT depth = 1.0f);
};
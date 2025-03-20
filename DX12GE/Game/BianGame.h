#pragma once

#include "../Engine/Game.h"
#include "../Engine/Window.h"
#include <DirectXMath.h>
#include "Camera.h" 

#include "KatamariGame.h" 

#include "LightManager.h"

#include "../Engine/UploadBuffer.h"

using namespace Microsoft::WRL;
using namespace DirectX;

class BianGame : public Game
{
public:
    using super = Game;

    BianGame(const std::wstring& name, int width, int height, bool vSync = false);
    
    // Загрузка контента, необходимого для рендера
    virtual bool LoadContent() override;

    // Выгрузить контент, загруженный в LoadContent
    virtual void UnloadContent() override;

    KatamariGame katamari;
protected:
    // Обновите игровую логику
    virtual void OnUpdate(UpdateEventArgs& e) override;

    // Рендер
    virtual void OnRender(RenderEventArgs& e) override;

    virtual void OnKeyPressed(KeyEventArgs& e) override;

    virtual void OnKeyReleased(KeyEventArgs& e) override;
    
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;

    virtual void OnMouseMoved(MouseMotionEventArgs& e) override;

    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;

    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;

    virtual void OnResize(ResizeEventArgs& e) override;


private:
    // Helper functions
    // Transition a resource
    void TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    // Clear a render target view
    void ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    // Clear the depth of a depth-stencil view.
    void ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);
    
    template<typename T>
    void SetGraphicsDynamicStructuredBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, const std::vector<T>& bufferData);

    // Resize the depth buffer to match the size of the client area.
    void ResizeDepthBuffer(int width, int height);

    uint64_t m_FenceValues[Window::BufferCount] = {};

    // Depth buffer.
    ComPtr<ID3D12Resource> m_DepthBuffer;
    // Descriptor heap for depth buffer.
    ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

    // Root signature
    ComPtr<ID3D12RootSignature> m_RootSignature;

    // Pipeline state object.
    ComPtr<ID3D12PipelineState> m_PipelineState;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    unique_ptr<UploadBuffer> m_UploadBuffer;

    LightManager lights;

    Camera m_Camera;
    bool m_ContentLoaded;
};


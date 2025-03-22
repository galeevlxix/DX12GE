#pragma once

#include "../Engine/Game.h"
#include "../Engine/Window.h"
#include "../Engine/Pipeline.h"
#include "../Engine/DepthBuffer.h"
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

    virtual void UnloadContent() override {};

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

    uint64_t m_FenceValues[Window::BufferCount] = {};

    Pipeline m_Pipeline;

    DepthBuffer m_DepthBuffer;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    unique_ptr<UploadBuffer> m_UploadBuffer;

    LightManager lights;

    Camera m_Camera;
};


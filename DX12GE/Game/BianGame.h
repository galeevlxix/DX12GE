#pragma once

#include "../Engine/Game.h"
#include "../Engine/Window.h"

#include <DirectXMath.h>

#include "BaseObject.h"

using namespace Microsoft::WRL;

class Racket
{
private:
    const float borderX = 18.0f;
    const float borderY = 10.0f;
    const static int length = 5;
    BaseObject cubes[length];

    bool CheckYBorder(float dy);
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, bool left);
    void OnUpdate(double deltaTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
    void Move(float dy);
    void SetStartPosition();

    Vector3 GetNewDirection(Vector3 ballPos, Vector3 direction);
};

class Ball
{
private:
    const float borderX = 18.0f;
    const float borderY = 10.0f;
    BaseObject cube;

    float speed = 8.0;
    Vector3 direction;

    void CheckYBorder();
    void CheckRackets(Racket* left, Racket* right);
    void CheckXBorder(Racket* left, Racket* right);
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void OnUpdate(double deltaTime, Racket *left, Racket * right);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
};

class BianGame : public Game
{
public:
    using super = Game;

    BianGame(const std::wstring& name, int width, int height, bool vSync = false);
    
    // Загрузка контента, необходимого для рендера
    virtual bool LoadContent() override;

    // Выгрузить контент, загруженный в LoadContent
    virtual void UnloadContent() override;
protected:
    // Обновите игровую логику
    virtual void OnUpdate(UpdateEventArgs& e) override;

    // Рендер
    virtual void OnRender(RenderEventArgs& e) override;

    virtual void OnKeyPressed(KeyEventArgs& e) override;

    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;

    virtual void OnResize(ResizeEventArgs& e) override;

private:
    // Helper functions
    // Transition a resource
    void TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    // Clear a render target view
    void ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    // Clear the depth of a depth-stencil view.
    void ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

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

    Racket lRacket;
    Racket rRacket;

    Ball ball;

    float m_FoV;

    //DirectX::XMMATRIX m_ModelMatrix;
    DirectX::XMMATRIX m_ViewMatrix;
    DirectX::XMMATRIX m_ProjectionMatrix;

    bool m_ContentLoaded;
};
#pragma once

#include "../Engine/Game.h"
#include "../Engine/Window.h"
#include "../Engine/Pipeline.h"
#include "../Engine/DepthBuffer.h"
#include "../Engine/UploadBuffer.h"
#include "../Engine/DebugSystem/DebugRenderSystem.h"
#include "../Engine/Shadows/ShadowMap.h"
#include "../Engine/Shadows/ShadowMapPipeline.h"

#include "Camera.h" 
#include "KatamariGame.h" 
#include "LightManager.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

class BianGame : public Game
{
public:
    using super = Game;
    
    BianGame(const wstring& name, int width, int height, bool vSync = false);
    ~BianGame();

    virtual bool LoadContent() override;
    virtual void UnloadContent() override {};

    Camera m_Camera;

protected:
    virtual void OnUpdate(UpdateEventArgs& e) override;
    virtual void OnRender(RenderEventArgs& e) override;
    virtual void OnKeyPressed(KeyEventArgs& e) override;
    virtual void OnKeyReleased(KeyEventArgs& e) override;
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
    virtual void OnMouseMoved(MouseMotionEventArgs& e) override;
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;
    virtual void OnResize(ResizeEventArgs& e) override;

private:
    void TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
    
    void ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    template<typename T>
    void SetGraphicsDynamicStructuredBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, const vector<T>& bufferData);

    template<typename T>
    void SetGraphicsConstants(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, const T& bufferData);

    void AddDebugObjects();

    uint64_t m_FenceValues[Window::BufferCount] = {};

    Pipeline m_Pipeline;
    ShadowMapPipeline m_ShadowMapPipeline;
    DepthBuffer m_DepthBuffer;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    unique_ptr<UploadBuffer> m_UploadBuffer;

    DebugRenderSystem debug;
    bool shouldAddDebugObjects = false;

    KatamariGame katamari;
    LightManager lights;

    unique_ptr<ShadowMap> m_ShadowMap;
    DirectX::BoundingSphere m_SceneBounds;
    Vector3 mLightPosW;

    Matrix mLightView;
    Matrix mLightProj;

    Matrix mShadowTransform;

    void UpdateShadowTransform();
    void UpdateShadowPassCB();
    void DrawSceneToShadowMap();
};


#pragma once

#include "../Engine/Game.h"
#include "../Engine/Window.h"
#include "../Engine/Pipeline.h"
#include "../Engine/DepthBuffer.h"
#include "../Engine/DebugSystem/DebugRenderSystem.h"
#include "../Engine/Shadows/CascadedShadowMap.h"
#include "../Engine/Shadows/ShadowMapPipeline.h"
#include "../Engine/GBuffer.h"
#include "../Engine/GeometryPassPipeline.h"
#include "../Engine/ParticleSystem/ParticlePipeline.h"
#include "../Engine/ParticleSystem/ParticleComputePipeline.h"
#include "../Engine/LightPassPipeline.h"

#include "Camera.h" 
#include "KatamariGame.h" 
#include "LightManager.h"
#include "../Engine/ParticleSystem/ParticleSystem.h"
#include "../Engine/Texture3D.h"

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
    void DrawDebugObjects();
    void DrawSceneToShadowMaps();
    void DrawSceneToGBuffer();
    void DrawParticlesToGBuffer();
    void LightPassRender(RenderEventArgs& e);
    void RefreshTitle(UpdateEventArgs& e);

    uint64_t m_FenceValues[Window::BufferCount] = {};

    Pipeline m_Pipeline;
    ShadowMapPipeline m_ShadowMapPipeline;
    DepthBuffer m_DepthBuffer;

    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    DebugRenderSystem debug;
    bool shouldAddDebugObjects = false;

    KatamariGame katamariScene;  
    LightManager lights;

    CascadedShadowMap m_CascadedShadowMap;

    GBuffer m_ParticleGBuffer;
    ParticlePipeline m_ParticlePipeline;
    ParticleComputePipeline m_ParticleComputePipeline;
    
    ParticleSystem particles;
    Texture3D tex3d;
    bool stopParticles = false;
    Vector3 boxPosition = Vector3(0, 0, 0);
    Vector3 boxSize = Vector3(30, 30, 30);

    GBuffer m_GBuffer;
    GeometryPassPipeline m_GeometryPassPipeline;
    LightPassPipeline m_LightPassPipeline;
};


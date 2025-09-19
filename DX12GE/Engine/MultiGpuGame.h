#pragma once

#include "Base/Application.h"
#include "Base/Game.h"
#include "Base/Window.h"
#include "Base/CommandQueue.h"
#include "Base/DescriptorHeaps.h"
#include "Base/CommandExecutor.h"

#include "Pipelines/Pipeline.h"
#include "Pipelines/ShadowMapPipeline.h"
#include "Pipelines/GeometryPassPipeline.h"
#include "Pipelines/ParticlePipeline.h"
#include "Pipelines/ParticleComputePipeline.h"
#include "Pipelines/LightPassPipeline.h"
#include "Pipelines/SSRPipeline.h"
#include "Pipelines/MergingPipeline.h"

#include "Graphics/ShaderResources.h"

#include "Graphics/DebugRenderSystem.h"
#include "Graphics/CascadedShadowMap.h"

#include "Graphics/Camera.h" 
#include "Graphics/LightManager.h"
#include "Graphics/ParticleSystem.h"
#include "Graphics/Texture3D.h"
#include "Graphics/TextureBuffer.h"
#include "Graphics/SSRCrossAdapterResources.h"
#include "../Game/KatamariGame.h" 

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

class MultiGpuGame : public Game
{
public:
    using super = Game;
    
    MultiGpuGame(const wstring& name, int width, int height, bool vSync = false);
    ~MultiGpuGame();

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
    void DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void DrawSceneToShadowMaps(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void DrawSceneToGBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void LightPassRender(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void DrawSSR(
        ComPtr<ID3D12GraphicsCommandList2> commandList,
        GraphicsAdapter graphicsAdapter,
        shared_ptr<DepthBuffer> depthBuffer,
        std::shared_ptr<GBuffer> gBuffer,
        std::shared_ptr<TextureBuffer> lightPassResult,
        std::shared_ptr<TextureBuffer> ssrResult,
        std::shared_ptr<SSRPipeline> ssrPipeline);
    void MergeResults(ComPtr<ID3D12GraphicsCommandList2> commandList);

    void DrawSingleGpu();
    void DrawMultiGpuTest();
    void DrawMultiGpu();

    void RefreshTitle(UpdateEventArgs& e);
    void DrawParticlesForward(ComPtr<ID3D12GraphicsCommandList2> commandList);

    ComPtr<ID3D12Device2> PrimaryDevice;
    ComPtr<ID3D12Device2> SecondDevice;

    uint64_t m_FenceValues[Window::BufferCount] = {};
    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    bool m_Initialized = false;
    
    bool m_IsMultiGpuRender = false;
    bool m_IsFirstFrame = true;

    bool IsTesting = false;
    vector<float> elapsed;
    void TestTime(string outputFile, float elapsedTime);

    CommandExecutor* executor;

    // SCENE

    DebugRenderSystem debug;
    bool shouldAddDebugObjects = false;

    KatamariGame katamariScene;  
    LightManager lights;

    CascadedShadowMap m_CascadedShadowMap;

    CrossAdapterTextureResources CATR;

    // PARTICLES

    ParticleSystem particles;
    Texture3D tex3d;
    bool stopParticles = false;
    Vector3 boxPosition = Vector3(50, 0, -15);
    Vector3 boxSize = Vector3(30, 30, 30);

    // PIPELINES

    ParticlePipeline m_ParticlePipeline;
    ParticleComputePipeline m_ParticleComputePipeline;
    SimplePipeline m_SimplePipeline;
    ShadowMapPipeline m_ShadowMapPipeline;
    GeometryPassPipeline m_GeometryPassPipeline;
    std::shared_ptr<SSRPipeline> m_SSRPipelinePrimaryDevice;
    std::shared_ptr<SSRPipeline> m_SSRPipelineSecondDevice;
    MergingPipeline m_MergingPipeline;
    LightPassPipeline m_LightPassPipeline;
};


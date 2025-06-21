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
#include "Graphics/DepthBuffer.h"
#include "Graphics/DebugRenderSystem.h"
#include "Graphics/CascadedShadowMap.h"
#include "Graphics/GBuffer.h"
#include "Graphics/Camera.h" 
#include "Graphics/LightManager.h"
#include "Graphics/ParticleSystem.h"
#include "Graphics/Texture3D.h"
#include "Graphics/TextureBuffer.h"
#include "Graphics/SharedMemory.h"

#include "../Game/KatamariGame.h" 

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

class BianEngineGame : public Game
{
public:
    using super = Game;
    
    BianEngineGame(const wstring& name, int width, int height, bool vSync = false);
    ~BianEngineGame();

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
    void DrawSceneToShadowMaps();
    void DrawSceneToGBuffer();
    void LightPassRender();
    void DrawSSR();
    void DrawSSRSecondDevice();
    void MergeResults();

    void CopyPrimaryDeviceDataToSharedMemory();
    void CopySecondDeviceDataToSharedMemory();
    void CopySharedMemoryDataToPrimaryDevice();
    void CopySharedMemoryDataToSecondDevice(); 

    void RefreshTitle(UpdateEventArgs& e);
    void DrawParticlesForward(ComPtr<ID3D12GraphicsCommandList2> commandList);

    uint64_t m_FenceValues[Window::BufferCount] = {};
    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    bool m_Initialized = false;

    bool IsTesting = false;
    vector<float> elapsed;
    void TestTime(string outputFile, float elapsedTime);

    // SCENE

    DepthBuffer m_PrimaryDeviceDepthBuffer;
    DepthBuffer m_SecondDeviceDepthBuffer;

    DebugRenderSystem debug;
    bool shouldAddDebugObjects = false;

    KatamariGame katamariScene;  
    LightManager lights;

    CascadedShadowMap m_CascadedShadowMap;
    
    GBuffer m_PrimaryDeviceGBuffer;
    std::shared_ptr<TextureBuffer> m_PrimaryDeviceLightPassResult;
    std::shared_ptr<TextureBuffer> m_PrimaryDeviceSSRResult;

    GBuffer m_SecondDeviceGBuffer;
    std::shared_ptr<TextureBuffer> m_SecondDeviceLightPassResult;
    std::shared_ptr<TextureBuffer> m_SecondDeviceSSRResult;

    std::unique_ptr<SharedMemory> m_SharedMemory;

    bool drawSSR = true;
    bool resizeSSR = false;

    CommandExecutor* executor;

    // PARTICLES

    ParticleSystem particles;
    Texture3D tex3d;
    bool stopParticles = false;
    Vector3 boxPosition = Vector3(50, 0, -15);
    Vector3 boxSize = Vector3(30, 30, 30);

    // PIPELINE

    ParticlePipeline m_ParticlePipeline;
    ParticleComputePipeline m_ParticleComputePipeline;
    SimplePipeline m_SimplePipeline;
    ShadowMapPipeline m_ShadowMapPipeline;
    GeometryPassPipeline m_GeometryPassPipeline;
    SSRPipeline m_SSRPipelinePrimaryDevice;
    SSRPipeline m_SSRPipelineSecondDevice;
    MergingPipeline m_MergingPipeline;
    LightPassPipeline m_LightPassPipeline;
};


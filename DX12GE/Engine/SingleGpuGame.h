#pragma once

#include <map>

#include "Base/Application.h"
#include "Base/Game.h"
#include "Base/Window.h"
#include "Base/CommandQueue.h"
#include "Base/SceneJsonSerializer.h"

#include "Pipelines/Pipeline.h"
#include "Pipelines/ShadowMapPipeline.h"
#include "Pipelines/GeometryPassPipeline.h"
#include "Pipelines/ParticlePipeline.h"
#include "Pipelines/ParticleComputePipeline.h"
#include "Pipelines/LightPassPipeline.h"
#include "Pipelines/SSRPipeline.h"
#include "Pipelines/MergingPipeline.h"
#include "Pipelines/SkyboxPipeline.h"

#include "Graphics/DescriptorHeaps.h"
#include "Graphics/ShaderResources.h"
#include "Graphics/Object3DEntity.h"
#include "Graphics/DebugRenderSystem.h"
#include "Graphics/CascadedShadowMap.h"
#include "Graphics/Camera.h" 
#include "Graphics/LightManager.h"
#include "Graphics/ParticleSystem.h"
#include "Graphics/Texture3D.h"
#include "Graphics/TextureBuffer.h"
#include "Graphics/DepthBuffer.h"
#include "Graphics/GBuffer.h"
#include "Graphics/Skybox.h"

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace std;

class SingleGpuGame : public Game
{
public:
    using super = Game;
    
    SingleGpuGame(const wstring& name, int width, int height, bool vSync = false);
    ~SingleGpuGame();

    virtual bool Initialize() override final;
    virtual bool LoadContent() override;
    virtual void UnloadContent() override;
    virtual void Destroy() override;

protected:
    virtual void OnUpdate(UpdateEventArgs& e) override;
    virtual void OnRender(RenderEventArgs& e) override final;
    virtual void OnKeyPressed(KeyEventArgs& e) override;
    virtual void OnKeyReleased(KeyEventArgs& e) override;
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
    virtual void OnMouseMoved(MouseMotionEventArgs& e) override;
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;
    virtual void OnResize(ResizeEventArgs& e) override;

public:
    Object3DEntity* Get(std::string name);
    void SaveSceneToFile();

private:
    void DrawSkybox(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void DrawParticles(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void DrawForwardObjects(ComPtr<ID3D12GraphicsCommandList2> commandList);

    void DrawSceneToShadowMaps(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void DrawSceneToGBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void LightPassRender(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void DrawSSR(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void MergeResults(ComPtr<ID3D12GraphicsCommandList2> commandList);

    void RefreshTitle(UpdateEventArgs& e);

    void UpdateSceneObjects(float deltaTime);
    void DrawSceneObjects(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);

private:

    ComPtr<ID3D12Device2> m_Device;

    uint64_t m_FenceValues[Window::BufferCount] = {};
    D3D12_VIEWPORT m_Viewport;
    D3D12_RECT m_ScissorRect;

    bool m_Initialized = false;
    
    bool m_IsTesting = false;
    vector<float> m_ElapsedTimeArray;
    const string m_TestTimeOutputFile = "../../DX12GE/Resources/single_gpu.txt";
    void TestTime(float elapsedTime);

    // SCENE

    Camera* m_Camera;
    map<string, Object3DEntity*> m_Objects;
    ThirdPersonPlayer* m_Player;
    SceneJsonSerializer m_SceneSerializer;
    bool m_SerializeSceneOnExit = false;

    bool m_ShouldAddDebugObjects = false;
    DebugRenderSystem m_DebugSystem;
    LightManager m_Lights;
    CascadedShadowMap m_CascadedShadowMap;

    std::shared_ptr<DepthBuffer> m_DepthBuffer;
    GBuffer m_GBuffer;
    std::shared_ptr<TextureBuffer> m_LightPassBuffer;
    std::shared_ptr<TextureBuffer> m_SSRBuffer;

    Skybox m_Skybox;

    // PARTICLES

    ParticleSystem m_ParticleSystem;
    Texture3D m_tex3d;
    bool m_stopParticles = false;
    Vector3 m_boxPosition = Vector3(50, 0, -15);
    Vector3 m_boxSize = Vector3(30, 30, 30);

    // PIPELINES

    ParticlePipeline m_ParticlePipeline;
    ParticleComputePipeline m_ParticleComputePipeline;
    SimplePipeline m_SimplePipeline;
    ShadowMapPipeline m_ShadowMapPipeline;
    GeometryPassPipeline m_GeometryPassPipeline;
    SSRPipeline m_SSRPipeline;
    MergingPipeline m_MergingPipeline;
    LightPassPipeline m_LightPassPipeline;    
    SkyboxPipeline m_SkyboxPipeline;
};
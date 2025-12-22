#include "SingleGpuGame.h"

#include "Graphics/ResourceStorage.h"
#include "Graphics/ShaderResources.h"

#include <sstream>
#include <string>
#include <chrono>
#include <fstream>

SingleGpuGame::SingleGpuGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))) { }

bool SingleGpuGame::Initialize()
{
    m_Device = Application::Get().GetPrimaryDevice();
    
    // INITIALIZE
    ShaderResources::Create(true);
    DescriptorHeaps::OnInit(m_Device, GraphicAdapterPrimary);

    // PIPELINES
    m_GeometryPassPipeline.Initialize(m_Device);
    m_LightPassPipeline.Initialize(m_Device);
    m_ParticlePipeline.Initialize(m_Device);
    m_ParticleComputePipeline.Initialize(m_Device);
    m_SimplePipeline.Initialize(m_Device);
    m_ShadowMapPipeline.Initialize(m_Device);
    m_SSRPipeline.Initialize(m_Device);
    m_MergingPipeline.Initialize(m_Device);
    m_SkyboxPipeline.Initialize(m_Device);

    // 3D SCENE
    m_CascadedShadowMap.Create(m_Device);

    m_GBuffer.Init(m_Device, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight());

    m_SSRBuffer = std::make_shared<TextureBuffer>();
    m_SSRBuffer->SetName(L"SSRResult");
    m_SSRBuffer->Init(m_Device, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, false);

    m_LightPassBuffer = std::make_shared<TextureBuffer>();
    m_LightPassBuffer->SetName(L"LightPass");
    m_LightPassBuffer->Init(m_Device, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, false);

    m_DepthBuffer = std::make_shared<DepthBuffer>();
    m_DepthBuffer->Init(GraphicAdapterPrimary);
    m_DepthBuffer->Resize(GetClientWidth(), GetClientHeight());

    if (!super::Initialize()) return false;
    
    return true;
}

bool SingleGpuGame::LoadContent()
{
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    Singleton::Initialize();
    Singleton::GetSelection()->SetTextureBuffer(m_GBuffer.GetBuffer(GBuffer::TargetType::ID));

    m_NodeGraph = Singleton::GetNodeGraph();
    m_NodeGraph->WindowRatio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());

    Singleton::GetSerializer()->Load(commandList);

    m_ParticleSystem.OnLoad(commandList);

    m_Skybox.OnLoad(commandList);

    // DRAW THE CUBE
    
    ShaderResources::GetParticleComputeCB()->BoxPosition = m_boxPosition;
    ShaderResources::GetParticleComputeCB()->BoxSize = Vector3(m_boxSize);

    // CREATE TEXTURE3D
    m_tex3d.Load(commandList, static_cast<int>(m_boxSize.x), static_cast<int>(m_boxSize.y), static_cast<int>(m_boxSize.z));

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_Initialized = true;
    return true;
}

void SingleGpuGame::OnUpdate(UpdateEventArgs& e)
{
    if (!m_Initialized || !Singleton::IsInitialized()) return;
    super::OnUpdate(e);

    Singleton::GetExecutor()->Update();

    float elapsedTime = static_cast<float>(e.ElapsedTime);

    m_NodeGraph->GetRoot()->OnUpdate(e.ElapsedTime);

    m_Lights.OnUpdate(elapsedTime);

    CameraNode* camera = m_NodeGraph->GetCurrentCamera();

    ShaderResources::GetWorldCB()->LightProps.CameraPos = Vector4(camera->GetWorldPosition());

    ShaderResources::GetSSRCB()->ViewProjection = camera->GetViewProjMatrix();
    ShaderResources::GetSSRCB()->CameraPos = Vector4(camera->GetWorldPosition());

    m_ParticleSystem.OnUpdate(elapsedTime, m_stopParticles, camera->GetViewProjMatrix(), camera->GetWorldPosition());
    m_CascadedShadowMap.Update(camera->GetWorldPosition(), ShaderResources::GetWorldCB()->DirLight.Direction);
    
    Singleton::GetDebugRender()->Clear();
    Singleton::GetSelection()->DrawDebug();

    RefreshTitle(e);
}

void SingleGpuGame::DrawSceneToShadowMaps(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Shadow);

    for (int i = 0; i < CASCADES_COUNT; i++)
    {
        ShadowMap* shadowMap = m_CascadedShadowMap.GetShadowMap(i);

        D3D12_VIEWPORT viewport = shadowMap->Viewport();
        D3D12_RECT rect = shadowMap->ScissorRect();
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = shadowMap->Dsv();

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &rect);

        TransitionResource(commandList, shadowMap->Resource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        m_ShadowMapPipeline.Set(commandList);

        commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

        DrawSceneObjectsForward(commandList, m_CascadedShadowMap.GetShadowViewProj(i));

        TransitionResource(commandList, shadowMap->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
}

void SingleGpuGame::DrawSceneToGBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Geometry);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_GBuffer.SetToWriteAndClear(commandList);
    m_DepthBuffer->ClearDepth(commandList);
    m_GBuffer.BindRenderTargets(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_GeometryPassPipeline.Set(commandList);
    
    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    DrawSceneObjectsForward(commandList, Singleton::GetNodeGraph()->GetCurrentCamera()->GetViewProjMatrix());

    m_GBuffer.SetToRead(commandList);
}
 
void SingleGpuGame::LightPassRender(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Lighting);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_LightPassBuffer->SetToWriteAndClear(commandList);
    m_LightPassBuffer->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_LightPassPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    ShaderResources::SetGraphicsShadowCB(commandList, 1);
    SetGraphicsDynamicStructuredBuffer(commandList, 2, Singleton::GetNodeGraph()->GetPointLightComponents());
    SetGraphicsDynamicStructuredBuffer(commandList, 3, Singleton::GetNodeGraph()->GetSpotLightComponents());
    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(4, commandList);

    m_GBuffer.SetGraphicsRootDescriptorTable(8,  GBuffer::POSITION, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(9,  GBuffer::NORMAL,   commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(10, GBuffer::DIFFUSE,  commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(11, GBuffer::EMISSIVE, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(12, GBuffer::ORM,      commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    DrawForwardOther(commandList);

    m_LightPassBuffer->SetToRead(commandList);
}

void SingleGpuGame::DrawSSR(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::SSR);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);
    
    m_SSRBuffer->SetToWriteAndClear(commandList);
    m_SSRBuffer->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_SSRPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    ShaderResources::SetSSRCB(commandList, 0, GraphicAdapterPrimary);
    m_GBuffer.SetGraphicsRootDescriptorTable(1, GBuffer::POSITION,  commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(2, GBuffer::NORMAL,    commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(3, GBuffer::ORM,       commandList);
    commandList->SetGraphicsRootDescriptorTable(4, m_LightPassBuffer->SrvGPU());
    m_Skybox.RenderTexture(commandList, 5);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    m_SSRBuffer->SetToRead(commandList);
}

void SingleGpuGame::MergeResults(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Merging);

    ComPtr<ID3D12Resource> backBuffer = m_pWindow->GetCurrentBackBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_pWindow->GetCurrentRenderTargetView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);
    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_MergingPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    commandList->SetGraphicsRootDescriptorTable(0, m_LightPassBuffer->SrvGPU());
    commandList->SetGraphicsRootDescriptorTable(1, m_SSRBuffer->SrvGPU());
    m_GBuffer.SetGraphicsRootDescriptorTable(2, GBuffer::ORM, commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void SingleGpuGame::DrawParticles(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::TransparentParticles);

    m_ParticleComputePipeline.SetUpdatePSO(commandList);
    m_tex3d.Render(commandList);
    m_ParticleSystem.OnUpdateComputeRender(commandList);

    m_ParticleComputePipeline.SetSortPSO(commandList);
    m_ParticleSystem.OnSortComputeRender(commandList);

    m_ParticlePipeline.Set(commandList);
    ShaderResources::SetGraphicsWorldCB(commandList, 2);
    m_ParticleSystem.OnRender(commandList);
}

void SingleGpuGame::DrawSkybox(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Skybox);

    m_SkyboxPipeline.Set(commandList);
    m_Skybox.OnRender(commandList, m_NodeGraph->GetCurrentCamera()->GetViewProjMatrixNoTranslation());
}

void SingleGpuGame::DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Debug);

    m_SimplePipeline.Set(commandList);    
    Singleton::GetDebugRender()->OnRender(commandList, m_NodeGraph->GetCurrentCamera()->GetViewProjMatrix());
}

void SingleGpuGame::DrawForwardOther(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    DrawDebugObjects(commandList);
    DrawSkybox(commandList);
    DrawParticles(commandList);
}

void SingleGpuGame::OnRender(RenderEventArgs& e)
{
    if (!m_Initialized || !Singleton::IsInitialized()) return;
    super::OnRender(e);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    ShaderResources::GetUploadBuffer()->Reset();

    {
        ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();
        DrawSceneToShadowMaps(commandList);
        DrawSceneToGBuffer(commandList);
        DrawSSR(commandList);
        uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
        commandQueue->WaitForFenceValue(fenceValue);
    }

    {
        ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();
        
        LightPassRender(commandList);
        MergeResults(commandList);

        m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
        currentBackBufferIndex = m_pWindow->Present();
        commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
    }

    Singleton::GetCurrentPass()->Set(CurrentPass::None);
}

void SingleGpuGame::OnKeyPressed(KeyEventArgs& e)
{
    Singleton::GetNodeGraph()->OnKeyPressed(e);

    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Quit(0);
        break;
    case KeyCode::F11:
        m_pWindow->ToggleFullscreen();
        break;
    case KeyCode::V:
        m_pWindow->ToggleVSync();
        break;
    case KeyCode::P:
        m_stopParticles = !m_stopParticles;
        break;    
    case KeyCode::B:
        if (Singleton::GetNodeGraph()->GetNodeByPath("root/fp_player") == Singleton::GetNodeGraph()->GetCurrentPlayer())
        {
            Singleton::GetNodeGraph()->GetNodeByPath("root/tp_player")->SetCurrent();
        }
        else
        {
            Singleton::GetNodeGraph()->GetNodeByPath("root/fp_player")->SetCurrent();
        }
        break;
    case KeyCode::R:
        auto commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto commandList = commandQueue->GetCommandList();
        m_ParticleSystem.SpawnParticleGroup(commandList, m_boxPosition + m_boxSize * 0.5f, 7.0f, 1000.0f);
        uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
        commandQueue->WaitForFenceValue(fenceValue);
        break;
    }    
}

void SingleGpuGame::OnKeyReleased(KeyEventArgs& e)
{
    Singleton::GetNodeGraph()->OnKeyReleased(e);
}

void SingleGpuGame::OnMouseWheel(MouseWheelEventArgs& e)
{
    Singleton::GetNodeGraph()->OnMouseWheel(e);
}

void SingleGpuGame::OnMouseMoved(MouseMotionEventArgs& e)
{
    Singleton::GetNodeGraph()->OnMouseMoved(e);
}

void SingleGpuGame::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    Singleton::GetNodeGraph()->OnMouseButtonPressed(e);
    Singleton::GetSelection()->OnMouseButtonPressed(e);
}

void SingleGpuGame::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    Singleton::GetNodeGraph()->OnMouseButtonReleased(e);
}

void SingleGpuGame::OnResize(ResizeEventArgs& e)
{
    if (e.Width == GetClientWidth() && e.Height == GetClientHeight()) return;
    
    super::OnResize(e);
    Singleton::GetNodeGraph()->OnResize(e);
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));
    m_DepthBuffer->Resize(e.Width, e.Height);
    m_GBuffer.Resize(e.Width, e.Height);
    m_LightPassBuffer->Resize(e.Width, e.Height);
    m_SSRBuffer->Resize(e.Width, e.Height);
    m_NodeGraph->WindowRatio = static_cast<float>(e.Width) / static_cast<float>(e.Height);
}

void SingleGpuGame::DrawSceneObjectsForward(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    UINT index = 1;
    for (auto obj : Singleton::GetNodeGraph()->GetAll3DObjects())
    {
        if (Singleton::GetCurrentPass()->Get() == CurrentPass::Geometry)
        {
            ShaderResources::GetGeometryPassCB()->ObjectId = index++;
        }
        obj.second->Render(commandList, viewProjMatrix);
    }
}

void SingleGpuGame::RefreshTitle(UpdateEventArgs& e)
{
    static unsigned long frameCounter = 0;
    static double timer = 0.0;

    static const std::wstring winName = L"BianGame";

    if (timer >= 1.0)
    {
        std::wstring fps = L" | Fps " + std::to_wstring(frameCounter);
        fps = Align(fps, 10);

        std::wstring mg = L" | Single-GPU";
        mg = Align(mg, 10);

        std::wstring cPos = L" | Pos " +
            rStr(m_NodeGraph->GetCurrentCamera()->GetWorldPosition().x, 1) + L"; " +
            rStr(m_NodeGraph->GetCurrentCamera()->GetWorldPosition().y, 1) + L"; " +
            rStr(m_NodeGraph->GetCurrentCamera()->GetWorldPosition().z, 1);
        cPos = Align(cPos, 21);

        std::wstring cTar = L" | Tar " +
            rStr(m_NodeGraph->GetCurrentCamera()->GetWorldDirection().x, 1) + L"; " +
            rStr(m_NodeGraph->GetCurrentCamera()->GetWorldDirection().y, 1) + L"; " +
            rStr(m_NodeGraph->GetCurrentCamera()->GetWorldDirection().z, 1);
        cTar = Align(cTar, 21);

        m_pWindow->UpdateWindowText(winName + mg + fps + cPos + cTar);

        timer = 0.0;
        frameCounter = 0;
    }
    else
    {
        frameCounter++;
        timer += e.ElapsedTime;
    }
}

void SingleGpuGame::UnloadContent()
{
    Singleton::Destroy();
}

void SingleGpuGame::Destroy()
{
    if (!m_Initialized) return;

    m_Initialized = false;

    m_CascadedShadowMap.Destroy();

    m_DepthBuffer->Destroy();
    m_DepthBuffer.reset();
    m_DepthBuffer = nullptr;

    m_GBuffer.Destroy();

    m_LightPassBuffer->Destroy();
    m_LightPassBuffer.reset();
    m_LightPassBuffer = nullptr;

    m_SSRBuffer->Destroy();
    m_SSRBuffer.reset();
    m_SSRBuffer = nullptr;

    m_ParticleSystem.Destroy();
    m_tex3d.Destroy();

    m_ParticlePipeline.Destroy();
    m_ParticleComputePipeline.Destroy();
    m_SimplePipeline.Destroy();
    m_ShadowMapPipeline.Destroy();
    m_GeometryPassPipeline.Destroy();
    m_SSRPipeline.Destroy();
    m_MergingPipeline.Destroy();
    m_LightPassPipeline.Destroy();

    ShaderResources::Destroy();
    DescriptorHeaps::DestroyAll();

    m_Device.Reset();
    m_Device = nullptr;

    super::Destroy();
}

SingleGpuGame::~SingleGpuGame()
{

}
#include "MultiGpuGame.h"

#include "Graphics/ResourceStorage.h"
#include "Graphics/ShaderResources.h"

#include <sstream>
#include <string>

#include <future>

MultiGpuGame::MultiGpuGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))) { }

bool MultiGpuGame::Initialize()
{
    m_PrimaryDevice = Application::Get().GetPrimaryDevice();
    m_SecondDevice = Application::Get().GetSecondDevice();

    // INITIALIZE
    ShaderResources::Create(true);
    DescriptorHeaps::OnInit(m_PrimaryDevice, GraphicAdapterPrimary);
    DescriptorHeaps::OnInit(m_SecondDevice, GraphicAdapterSecond);

    // PIPELINES
    m_GeometryPassPipeline.Initialize(m_PrimaryDevice);
    m_LightPassPipeline.Initialize(m_PrimaryDevice);
    m_ParticlePipeline.Initialize(m_PrimaryDevice);
    m_ParticleComputePipeline.Initialize(m_PrimaryDevice);
    m_SimplePipeline.Initialize(m_PrimaryDevice);
    m_ShadowMapPipeline.Initialize(m_PrimaryDevice);
    m_SSRPipeline.Initialize(m_SecondDevice);
    m_MergingPipeline.Initialize(m_PrimaryDevice);
    m_SkyboxPipeline.Initialize(m_PrimaryDevice);

    // 3D SCENE
    m_CascadedShadowMap.Create(m_PrimaryDevice);

    // PRIMARY DEVICE RESOURCES

    m_CATR.PrimaryDevice = m_PrimaryDevice;

    m_CATR.PrimaryGBuffer = std::make_shared<GBuffer>();
    m_CATR.PrimaryGBuffer->Init(m_PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight());

    m_CATR.PrimarySSRResult = std::make_shared<TextureBuffer>();
    m_CATR.PrimarySSRResult->SetName(L"SSRResultPrimaryResource");
    m_CATR.PrimarySSRResult->Init(m_PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, false);

    m_CATR.PrimaryLightPassResult = std::make_shared<TextureBuffer>();
    m_CATR.PrimaryLightPassResult->SetName(L"LightPassPrimaryResource");
    m_CATR.PrimaryLightPassResult->Init(m_PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, false);

    m_CATR.PrimaryDepthBuffer = std::make_shared<DepthBuffer>();
    m_CATR.PrimaryDepthBuffer->Init(GraphicAdapterPrimary);
    m_CATR.PrimaryDepthBuffer->Resize(GetClientWidth(), GetClientHeight());

    // SECOND DEVICE RESOURCES

	m_CATR.SecondDevice = m_SecondDevice; 

    m_CATR.SecondGBuffer = std::make_shared<GBuffer>();
    m_CATR.SecondGBuffer->Init(m_SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight());
           
    m_CATR.SecondSSRResult = std::make_shared<TextureBuffer>();
    m_CATR.SecondSSRResult->SetName(L"SSRResultSecondResource");
    m_CATR.SecondSSRResult->Init(m_SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, false);
           
    m_CATR.SecondLightPassResult = std::make_shared<TextureBuffer>();
    m_CATR.SecondLightPassResult->SetName(L"LightPassSecondResource");
    m_CATR.SecondLightPassResult->Init(m_SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM, false);
           
    m_CATR.SecondDepthBuffer = std::make_shared<DepthBuffer>();
    m_CATR.SecondDepthBuffer->Init(GraphicAdapterSecond);
    m_CATR.SecondDepthBuffer->Resize(GetClientWidth(), GetClientHeight());

    // CATR
    
    m_CATR.Initialize();

    // COMMON

    Singleton::Initialize();
    Singleton::GetSelection()->SetTextureBuffer(m_CATR.PrimaryGBuffer->GetBuffer(GBuffer::TargetType::ID));
    Singleton::GetNodeGraph()->WindowRatio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());

    test = new TestTime();

    if (!super::Initialize()) return false;
    
    return true;
}

bool MultiGpuGame::LoadContent()
{
    // PRIMARY DEVICE RESOURCES 

    shared_ptr<CommandQueue> primaryCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> primaryCommandList = primaryCommandQueue->GetCommandList();
    
    Singleton::GetSerializer()->Load(primaryCommandList);

    m_ParticleSystem.OnLoad(primaryCommandList);
    
    ShaderResources::GetParticleComputeCB()->BoxPosition = m_boxPosition;
    ShaderResources::GetParticleComputeCB()->BoxSize = Vector3(m_boxSize);

    m_tex3d.Load(primaryCommandList, static_cast<int>(m_boxSize.x), static_cast<int>(m_boxSize.y), static_cast<int>(m_boxSize.z));

    uint64_t primaryFenceValue = primaryCommandQueue->ExecuteCommandList(primaryCommandList);
    primaryCommandQueue->WaitForFenceValue(primaryFenceValue);

    // SECOND DEVICE RESOURCES 
    
    shared_ptr<CommandQueue> secondCommandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> secondCommandList = secondCommandQueue->GetCommandList();

    if (SkyBoxNode* skybox = Singleton::GetNodeGraph()->GetCurrentSkyBox())
    {
        auto skyTex = ResourceStorage::GetTexture(skybox->GetComponentId());
        uint32_t secSkyId = ResourceStorage::AddTexture(skyTex->GetResourcePath() + "_SecondGpu");
        m_SkyboxSecondDevice = ResourceStorage::GetTexture(secSkyId);
        m_SkyboxSecondDevice->OnLoadCubemap(secondCommandList, skyTex->GetResourcePath(), m_SecondDevice, GraphicAdapterSecond);
    }    

    uint64_t secondFenceValue = secondCommandQueue->ExecuteCommandList(secondCommandList);
    secondCommandQueue->WaitForFenceValue(secondFenceValue);

    m_Initialized = true;
    return true;
}

void MultiGpuGame::OnUpdate(UpdateEventArgs& e)
{
    if (!m_Initialized || !Singleton::IsInitialized()) return;
    super::OnUpdate(e);

    Singleton::GetExecutor()->Update();

    float elapsedTime = static_cast<float>(e.ElapsedTime);

    Singleton::GetNodeGraph()->GetRoot()->OnUpdate(e.ElapsedTime);

    m_Lights.OnUpdate(elapsedTime);

    CameraNode* camera = Singleton::GetNodeGraph()->GetCurrentCamera();
	const Matrix viewProj = camera->GetViewProjMatrix();
	const Vector3& cameraPos = camera->GetWorldPosition(); 

    ShaderResources::GetWorldCB()->LightProps.CameraPos = Vector4(cameraPos);

    ShaderResources::GetSSRCB()->ViewProjection = viewProj;
    ShaderResources::GetSSRCB()->CameraPos = Vector4(cameraPos);

    m_ParticleSystem.OnUpdate(elapsedTime, m_stopParticles, viewProj, cameraPos);
    m_CascadedShadowMap.Update(cameraPos, ShaderResources::GetWorldCB()->DirLight.Direction);
    
    Singleton::GetDebugRender()->Clear();
    Singleton::GetSelection()->DrawDebug();

    RefreshTitle(e);
}

void MultiGpuGame::DrawSceneToShadowMaps()
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Shadow);

    test->BeginPass();

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

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

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::Shadow);
}

void MultiGpuGame::DrawSceneToGBuffer()
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Geometry);

    test->BeginPass();

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_CATR.PrimaryDepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_CATR.PrimaryGBuffer->SetToWriteAndClear(commandList);
    m_CATR.PrimaryDepthBuffer->ClearDepth(commandList);
    m_CATR.PrimaryGBuffer->BindRenderTargets(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_GeometryPassPipeline.Set(commandList);
    
    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    DrawSceneObjectsForward(commandList, Singleton::GetNodeGraph()->GetCurrentCamera()->GetViewProjMatrix());

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::Geometry);
}
 
void MultiGpuGame::LightPassRender()
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Lighting);

    test->BeginPass();

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_CATR.PrimaryDepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_CATR.PrimaryLightPassResult->SetToWriteAndClear(commandList);
    m_CATR.PrimaryLightPassResult->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_LightPassPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    ShaderResources::SetGraphicsShadowCB(commandList, 1);
    SetGraphicsDynamicStructuredBuffer(commandList, 2, Singleton::GetNodeGraph()->GetPointLightComponents());
    SetGraphicsDynamicStructuredBuffer(commandList, 3, Singleton::GetNodeGraph()->GetSpotLightComponents());

    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(4, commandList);

    m_CATR.PrimaryGBuffer->SetToRead(commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(8,  GBuffer::POSITION, commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(9,  GBuffer::NORMAL,   commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(10, GBuffer::DIFFUSE,  commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(11, GBuffer::EMISSIVE, commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(12, GBuffer::ORM,      commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    m_CATR.PrimaryGBuffer->GetBuffer(GBuffer::POSITION)->SetToCommon(commandList);
    m_CATR.PrimaryGBuffer->GetBuffer(GBuffer::NORMAL)->SetToCommon(commandList);
    m_CATR.PrimaryGBuffer->GetBuffer(GBuffer::ORM)->SetToCommon(commandList);

    DrawDebugObjects(commandList);
    DrawSkybox(commandList);
    DrawParticles(commandList);    

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::Lighting);
}

void MultiGpuGame::DrawParticles(ComPtr<ID3D12GraphicsCommandList2> commandList)
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

void MultiGpuGame::DrawSkybox(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Skybox);

    m_SkyboxPipeline.Set(commandList);

    if (SkyBoxNode* skybox = Singleton::GetNodeGraph()->GetCurrentSkyBox())
    {
        skybox->Render(commandList, Singleton::GetNodeGraph()->GetCurrentCamera()->GetViewProjMatrixNoTranslation());
    }
}

void MultiGpuGame::DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Debug);

    m_SimplePipeline.Set(commandList);
    Singleton::GetDebugRender()->OnRender(commandList, Singleton::GetNodeGraph()->GetCurrentCamera()->GetViewProjMatrix());
}

void MultiGpuGame::DrawSSR()
{
    test->BeginPass();

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_CATR.SecondDepthBuffer->dsvCpuHandleIndex, GraphicAdapterSecond);
    
    m_CATR.SecondSSRResult->SetToWriteAndClear(commandList);
    m_CATR.SecondSSRResult->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_SSRPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterSecond).GetAddressOf());

    ShaderResources::SetSSRCB(commandList, 0, GraphicAdapterSecond);

	m_CATR.SecondGBuffer->GetBuffer(GBuffer::TargetType::POSITION)->SetToRead(commandList);
    m_CATR.SecondGBuffer->SetGraphicsRootDescriptorTable(1, GBuffer::POSITION, commandList);

    m_CATR.SecondGBuffer->GetBuffer(GBuffer::TargetType::NORMAL)->SetToRead(commandList);
    m_CATR.SecondGBuffer->SetGraphicsRootDescriptorTable(2, GBuffer::NORMAL, commandList);

    m_CATR.SecondGBuffer->GetBuffer(GBuffer::TargetType::ORM)->SetToRead(commandList);
    m_CATR.SecondGBuffer->SetGraphicsRootDescriptorTable(3, GBuffer::ORM, commandList);

	m_CATR.SecondLightPassResult->SetToRead(commandList);
    commandList->SetGraphicsRootDescriptorTable(4, m_CATR.SecondLightPassResult->SrvGPU());

    if (SkyBoxNode* skybox = Singleton::GetNodeGraph()->GetCurrentSkyBox())
    {
        m_SkyboxSecondDevice->OnRender(commandList, 5);
    }    

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    m_CATR.SecondSSRResult->SetToCommon(commandList);

    m_CATR.SecondGBuffer->GetBuffer(GBuffer::TargetType::POSITION)->SetToCommon(commandList);
    m_CATR.SecondGBuffer->GetBuffer(GBuffer::TargetType::NORMAL)->SetToCommon(commandList);
    m_CATR.SecondGBuffer->GetBuffer(GBuffer::TargetType::ORM)->SetToCommon(commandList);
    m_CATR.SecondLightPassResult->SetToCommon(commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::SSR);
}

void MultiGpuGame::MergeResults()
{
    Singleton::GetCurrentPass()->Set(CurrentPass::Merging);

    test->BeginPass();

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    ShaderResources::GetUploadBuffer()->Reset();

    ComPtr<ID3D12Resource> backBuffer = m_pWindow->GetCurrentBackBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_pWindow->GetCurrentRenderTargetView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_CATR.PrimaryDepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);
    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_MergingPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

	m_CATR.PrimaryLightPassResult->SetToRead(commandList);
    commandList->SetGraphicsRootDescriptorTable(0, m_CATR.PrimaryLightPassResult->SrvGPU());

    m_CATR.PrimarySSRResult->SetToRead(commandList);
    commandList->SetGraphicsRootDescriptorTable(1, m_CATR.PrimarySSRResult->SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    
    m_CATR.PrimaryLightPassResult->SetToCommon(commandList);
    m_CATR.PrimarySSRResult->SetToCommon(commandList);  

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    m_pWindow->Present();
    commandQueue->WaitForFenceValue(fenceValue);

    test->EndPass(CurrentPass::Merging);
}

void MultiGpuGame::RenderPrimaryGPU()
{
    if (!m_IsFirstFrame)
    {
        m_CATR.CopySharedMemoryDataToPrimaryDevice(test);
    }
    DrawSceneToShadowMaps();
    DrawSceneToGBuffer();
    LightPassRender();
    MergeResults();
    m_CATR.CopyPrimaryDeviceDataToSharedMemory(test);
}

void MultiGpuGame::RenderSecondGPU()
{
    if (m_IsFirstFrame) return;
    m_CATR.CopySharedMemoryDataToSecondDevice(test);
    DrawSSR();
    m_CATR.CopySecondDeviceDataToSharedMemory(test);
}

void MultiGpuGame::OnRender(RenderEventArgs& e)
{
    if (!m_Initialized || !Singleton::IsInitialized()) return;
    super::OnRender(e);

    test->BeginFrame();

    auto primaryFuture = std::async(std::launch::async, &MultiGpuGame::RenderPrimaryGPU, this);
    auto secondaryFuture = std::async(std::launch::async, &MultiGpuGame::RenderSecondGPU, this);
    
    // Ждем завершения обоих
    primaryFuture.get();
    secondaryFuture.get();

    test->EndFrame();
    m_IsFirstFrame = false;

    Singleton::GetCurrentPass()->Set(CurrentPass::None);
}

void MultiGpuGame::OnKeyPressed(KeyEventArgs& e)
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
        if (Singleton::GetNodeGraph()->GetNodeByPath("root/player_fp") == Singleton::GetNodeGraph()->GetCurrentPlayer())
        {
            Singleton::GetNodeGraph()->GetNodeByPath("root/player_tp")->SetCurrent();
        }
        else
        {
            Singleton::GetNodeGraph()->GetNodeByPath("root/player_fp")->SetCurrent();
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

void MultiGpuGame::OnKeyReleased(KeyEventArgs& e)
{
    Singleton::GetNodeGraph()->OnKeyReleased(e);
}

void MultiGpuGame::OnMouseWheel(MouseWheelEventArgs& e)
{
    Singleton::GetNodeGraph()->OnMouseWheel(e);
}

void MultiGpuGame::OnMouseMoved(MouseMotionEventArgs& e)
{
    Singleton::GetNodeGraph()->OnMouseMoved(e);
}

void MultiGpuGame::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    Singleton::GetNodeGraph()->OnMouseButtonPressed(e);
    Singleton::GetSelection()->OnMouseButtonPressed(e);
}

void MultiGpuGame::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    Singleton::GetNodeGraph()->OnMouseButtonReleased(e);
}

void MultiGpuGame::OnResize(ResizeEventArgs& e)
{
    if (e.Width == GetClientWidth() && e.Height == GetClientHeight()) return;
    
    super::OnResize(e);
    Singleton::GetNodeGraph()->OnResize(e);
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));
	m_CATR.Resize(e.Width, e.Height);
}

void MultiGpuGame::DrawSceneObjectsForward(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
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

void MultiGpuGame::RefreshTitle(UpdateEventArgs& e)
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
            rStr(Singleton::GetNodeGraph()->GetCurrentCamera()->GetWorldPosition().x, 1) + L"; " +
            rStr(Singleton::GetNodeGraph()->GetCurrentCamera()->GetWorldPosition().y, 1) + L"; " +
            rStr(Singleton::GetNodeGraph()->GetCurrentCamera()->GetWorldPosition().z, 1);
        cPos = Align(cPos, 21);

        std::wstring cTar = L" | Tar " +
            rStr(Singleton::GetNodeGraph()->GetCurrentCamera()->GetWorldDirection().x, 1) + L"; " +
            rStr(Singleton::GetNodeGraph()->GetCurrentCamera()->GetWorldDirection().y, 1) + L"; " +
            rStr(Singleton::GetNodeGraph()->GetCurrentCamera()->GetWorldDirection().z, 1);
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

void MultiGpuGame::UnloadContent()
{
    Singleton::Destroy();
}

void MultiGpuGame::Destroy()
{
    if (!m_Initialized) return;

    m_Initialized = false;

    m_CascadedShadowMap.Destroy();

    m_CATR.Destroy();

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

    m_PrimaryDevice.Reset();
    m_PrimaryDevice = nullptr;

    super::Destroy();
}

MultiGpuGame::~MultiGpuGame()
{

}
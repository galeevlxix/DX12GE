#include "MultiGpuGame.h"
#include "Graphics/ResourceStorage.h"

#include <sstream>
#include <string>
#include <chrono>
#include <fstream>

MultiGpuGame::MultiGpuGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))) { }

bool MultiGpuGame::Initialize()
{
    if (!super::Initialize()) return false;

    m_PrimaryDevice = Application::Get().GetPrimaryDevice();
    m_SecondDevice = Application::Get().GetSecondDevice();

    // INITIALIZE
    ShaderResources::Create(false);
    DescriptorHeaps::OnInit(m_PrimaryDevice, GraphicAdapterPrimary);
    DescriptorHeaps::OnInit(m_SecondDevice, GraphicAdapterSecond);

    // PIPELINES
    m_GeometryPassPipeline.Initialize(m_PrimaryDevice);
    m_LightPassPipeline.Initialize(m_PrimaryDevice);
    m_ParticlePipeline.Initialize(m_PrimaryDevice);
    m_ParticleComputePipeline.Initialize(m_PrimaryDevice);
    m_SimplePipeline.Initialize(m_PrimaryDevice);
    m_ShadowMapPipeline.Initialize(m_PrimaryDevice);
    m_SecondDevice_SSRPipeline.Initialize(m_SecondDevice);
    m_MergingPipeline.Initialize(m_PrimaryDevice);

    // 3D SCENE
    m_CascadedShadowMap.Create(m_PrimaryDevice);

    // PRIMARY DEVICE RESOURCES

    m_CATR.PrimaryDevice = m_PrimaryDevice;

    m_CATR.PrimaryGBuffer = std::make_shared<GBuffer>();
    m_CATR.PrimaryGBuffer->Init(m_PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight());

    m_CATR.PrimarySSRResult = std::make_shared<TextureBuffer>();
    m_CATR.PrimarySSRResult->SetName(L"SSRResultPrimaryResource");
    m_CATR.PrimarySSRResult->Init(m_PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    m_CATR.PrimaryLightPassResult = std::make_shared<TextureBuffer>();
    m_CATR.PrimaryLightPassResult->SetName(L"LightPassPrimaryResource");
    m_CATR.PrimaryLightPassResult->Init(m_PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    m_CATR.PrimaryDepthBuffer = std::make_shared<DepthBuffer>();
    m_CATR.PrimaryDepthBuffer->Init(GraphicAdapterPrimary);
    m_CATR.PrimaryDepthBuffer->Resize(GetClientWidth(), GetClientHeight());

    // SECOND DEVICE RESOURCES

    m_CATR.SecondDevice = m_SecondDevice;

    m_CATR.SecondGBuffer = std::make_shared<GBuffer>();
    m_CATR.SecondGBuffer->Init(m_SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight());

    m_CATR.SecondSSRResult = std::make_shared<TextureBuffer>();
    m_CATR.SecondSSRResult->SetName(L"SSRResultSecondResource");
    m_CATR.SecondSSRResult->Init(m_SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    m_CATR.SecondLightPassResult = std::make_shared<TextureBuffer>();
    m_CATR.SecondLightPassResult->SetName(L"LightPassSecondResource");
    m_CATR.SecondLightPassResult->Init(m_SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    m_CATR.SecondDepthBuffer = std::make_shared<DepthBuffer>();
    m_CATR.SecondDepthBuffer->Init(GraphicAdapterSecond);
    m_CATR.SecondDepthBuffer->Resize(GetClientWidth(), GetClientHeight());
    
    // CATR

    m_CATR.Initialize();

    return true;
}

bool MultiGpuGame::LoadContent()
{
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    m_Camera = new Camera();
    m_Camera->OnLoad();
    m_Camera->Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());

    m_SceneSerializer.Load(commandList, m_Objects);
    m_Player = dynamic_cast<ThirdPersonPlayer*>(m_Objects["player"]);
    m_Player->SetCamera(m_Camera);

    m_ParticleSystem.OnLoad(commandList);
    m_Lights.Init(m_Player);

    ShaderResources::GetSSRCB()->MaxDistance = 32.0f;
    ShaderResources::GetSSRCB()->RayStep = 0.5f;
    ShaderResources::GetSSRCB()->Thickness = 0.25f;
    
    // DRAW THE CUBE
    m_DebugSystem.DrawPoint(m_boxPosition, 2.0f);

    BoundingBox box(m_boxPosition + m_boxSize * 0.5f, m_boxSize * 0.5f);
    m_DebugSystem.DrawBoundingBox(box);

    m_DebugSystem.Update(commandList);

    ShaderResources::GetParticleComputeCB()->BoxPosition = m_boxPosition;
    ShaderResources::GetParticleComputeCB()->BoxSize = Vector3(m_boxSize);
    
    // CREATE TEXTURE3D
    m_tex3d.Load(commandList, static_cast<int>(m_boxSize.x), static_cast<int>(m_boxSize.y), static_cast<int>(m_boxSize.z));

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_Initialized = true;
    return true;
}

Object3DEntity* MultiGpuGame::Get(std::string name)
{
    if (m_Objects.find(name) == m_Objects.end()) return nullptr;
    return m_Objects[name];
}

void MultiGpuGame::SaveSceneToFile()
{
    m_SceneSerializer.Save(m_Objects);
}

void MultiGpuGame::OnUpdate(UpdateEventArgs& e)
{
    if (!m_Initialized) return;
    super::OnUpdate(e);
    float elapsedTime = static_cast<float>(e.ElapsedTime);
    UpdateSceneObjects(elapsedTime);
    m_Lights.OnUpdate(elapsedTime);
    ShaderResources::GetWorldCB()->LightProps.CameraPos = m_Camera->Position;
    ShaderResources::GetSSRCB()->ViewProjection = m_Camera->GetViewProjMatrix();
    ShaderResources::GetSSRCB()->CameraPos = m_Camera->Position;
    m_ParticleSystem.OnUpdate(elapsedTime, m_stopParticles, m_Camera->GetViewProjMatrix(), m_Camera->Position);
    RefreshTitle(e);
    m_CascadedShadowMap.Update(m_Camera->Position, ShaderResources::GetWorldCB()->DirLight.Direction);
}

void MultiGpuGame::DrawSceneToShadowMaps(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Shadow);

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

        DrawSceneObjects(commandList, m_CascadedShadowMap.GetShadowViewProj(i));

        TransitionResource(commandList, shadowMap->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
}

void MultiGpuGame::DrawSceneToGBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Geometry);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_CATR.PrimaryDepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_CATR.PrimaryGBuffer->SetToWriteAndClear(commandList);
    m_CATR.PrimaryDepthBuffer->ClearDepth(commandList);
    m_CATR.PrimaryGBuffer->BindRenderTargets(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_GeometryPassPipeline.Set(commandList);
    
    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    DrawSceneObjects(commandList, m_Camera->GetViewProjMatrix());

    m_CATR.PrimaryGBuffer->SetToRead(commandList);
}
 
void MultiGpuGame::LightPassRender(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Lighting);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_CATR.PrimaryDepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_CATR.PrimaryLightPassResult->SetToWriteAndClear(commandList);
    m_CATR.PrimaryLightPassResult->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_LightPassPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    ShaderResources::SetGraphicsShadowCB(commandList, 1);
    SetGraphicsDynamicStructuredBuffer(commandList, 2, m_Lights.m_PointLights);
    SetGraphicsDynamicStructuredBuffer(commandList, 3, m_Lights.m_SpotLights);
    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(4, commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(8, GBuffer::POSITION, commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(9, GBuffer::NORMAL, commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(10, GBuffer::DIFFUSE, commandList);
    m_CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(11, GBuffer::EMISSIVE, commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    DrawDebugObjects(commandList);
    DrawParticlesForward(commandList);

    m_CATR.PrimaryLightPassResult->SetToRead(commandList);
}

void MultiGpuGame::DrawSSR(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::SSR);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_CATR.SecondDepthBuffer->dsvCpuHandleIndex, GraphicAdapterSecond);
    
    m_CATR.SecondSSRResult->SetToWriteAndClear(commandList);
    m_CATR.SecondSSRResult->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_SecondDevice_SSRPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterSecond).GetAddressOf());

    ShaderResources::SetSSRCB(commandList, 0, GraphicAdapterSecond);
    m_CATR.SecondGBuffer->SetGraphicsRootDescriptorTable(1, GBuffer::POSITION, commandList);
    m_CATR.SecondGBuffer->SetGraphicsRootDescriptorTable(2, GBuffer::NORMAL, commandList);
    m_CATR.SecondGBuffer->SetGraphicsRootDescriptorTable(3, GBuffer::ORM, commandList);
    commandList->SetGraphicsRootDescriptorTable(4, m_CATR.SecondLightPassResult->SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    m_CATR.SecondSSRResult->SetToRead(commandList);
}

void MultiGpuGame::MergeResults(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Merging);

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

    commandList->SetGraphicsRootDescriptorTable(0, m_CATR.PrimaryLightPassResult->SrvGPU());
    commandList->SetGraphicsRootDescriptorTable(1, m_CATR.PrimarySSRResult->SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void MultiGpuGame::DrawParticlesForward(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::TransparentParticles);

    m_ParticleComputePipeline.SetUpdatePSO(commandList);
    m_tex3d.Render(commandList);
    m_ParticleSystem.OnUpdateComputeRender(commandList);

    m_ParticleComputePipeline.SetSortPSO(commandList);
    m_ParticleSystem.OnSortComputeRender(commandList);

    m_ParticlePipeline.Set(commandList);
    ShaderResources::SetGraphicsWorldCB(commandList, 2);
    m_ParticleSystem.OnRender(commandList);
}

void MultiGpuGame::DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Debug);

    m_SimplePipeline.Set(commandList);
    m_DebugSystem.Draw(commandList, m_Camera->GetViewProjMatrix());
}


void MultiGpuGame::OnRender(RenderEventArgs& e)
{
    if (!m_Initialized) return;
    super::OnRender(e);

    TestTime(static_cast<float>(e.ElapsedTime));

    // primary device

    shared_ptr<CommandQueue> primaryDeviceCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> primaryDeviceCommandList = primaryDeviceCommandQueue->GetCommandList();
    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();

    if (!m_IsFirstFrame)
        m_CATR.CopySharedMemoryDataToPrimaryDevice(primaryDeviceCommandList);

    DrawSceneToShadowMaps(primaryDeviceCommandList);
    DrawSceneToGBuffer(primaryDeviceCommandList);
    LightPassRender(primaryDeviceCommandList);
    m_CATR.CopyPrimaryDeviceDataToSharedMemory(primaryDeviceCommandList);

    MergeResults(primaryDeviceCommandList);

    uint64_t fenceValuePrimary = primaryDeviceCommandQueue->ExecuteCommandList(primaryDeviceCommandList);

    // second device

    shared_ptr<CommandQueue> secondDeviceCommandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> secondDeviceCommandList = secondDeviceCommandQueue->GetCommandList();
    if (!m_IsFirstFrame)
    {
        m_CATR.CopySharedMemoryDataToSecondDevice(secondDeviceCommandList);
        DrawSSR(secondDeviceCommandList);
        m_CATR.CopySecondDeviceDataToSharedMemory(secondDeviceCommandList);
    }
    uint64_t fenceValueSecond = secondDeviceCommandQueue->ExecuteCommandList(secondDeviceCommandList);

    // wait

    primaryDeviceCommandQueue->WaitForFenceValue(fenceValuePrimary);
    secondDeviceCommandQueue->WaitForFenceValue(fenceValueSecond);
    m_pWindow->Present();

    m_IsFirstFrame = false;

    CurrentPass::Set(CurrentPass::None);
}

void MultiGpuGame::TestTime(float elapsedTime)
{
    if (!m_IsTesting) return;

    if (m_Player->IsTesting())
    {
        m_ElapsedTimeArray.push_back(elapsedTime);
    }
    else
    {
        m_IsTesting = false;
        ofstream out(m_TestTimeOutputFile);
        for (size_t i = 0; i < m_ElapsedTimeArray.size(); i++)
        {
            out << m_ElapsedTimeArray[i] << endl;
        }
        m_ElapsedTimeArray.clear();
        out.close();
    }
}

void MultiGpuGame::OnKeyPressed(KeyEventArgs& e)
{
    super::OnKeyPressed(e);
    m_Player->OnKeyPressed(e);

    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Quit(0);
        break;
    case KeyCode::Enter:
        if (e.Alt)
        {
    case KeyCode::F11:
        m_pWindow->ToggleFullscreen();
        break;
        }
    case KeyCode::V:
        m_pWindow->ToggleVSync();
        break;
    case KeyCode::X:
        m_DebugSystem.canDraw = !m_DebugSystem.canDraw;
        m_ShouldAddDebugObjects = true;
        break;
    case KeyCode::P:
        m_stopParticles = !m_stopParticles;
        break;    
    case KeyCode::T:
        m_Player->StartTest();
        m_IsTesting = true;
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
    m_Player->OnKeyReleased(e);
}

void MultiGpuGame::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_Player->OnMouseWheel(e);
}

void MultiGpuGame::OnMouseMoved(MouseMotionEventArgs& e)
{
    m_Player->OnMouseMoved(e);
}

void MultiGpuGame::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    m_Player->OnMouseButtonPressed(e);
}

void MultiGpuGame::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    m_Player->OnMouseButtonReleased(e);
}

void MultiGpuGame::OnResize(ResizeEventArgs& e)
{
    if (e.Width == GetClientWidth() && e.Height == GetClientHeight()) return;
    super::OnResize(e);
    
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));
    m_CATR.Resize(e.Width, e.Height);
    m_Camera->Ratio = static_cast<float>(e.Width) / static_cast<float>(e.Height);
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

        std::wstring mg = L" | Multi-GPU";
        mg = Align(mg, 10);

        std::wstring cPos = L" | Pos " +
            rStr(m_Camera->Position.m128_f32[0], 1) + L"; " +
            rStr(m_Camera->Position.m128_f32[1], 1) + L"; " +
            rStr(m_Camera->Position.m128_f32[2], 1);
        cPos = Align(cPos, 21);  

        std::wstring cTar = L" | Tar " +
            rStr(m_Camera->Target.m128_f32[0], 1) + L"; " +
            rStr(m_Camera->Target.m128_f32[1], 1) + L"; " +
            rStr(m_Camera->Target.m128_f32[2], 1);
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

void MultiGpuGame::UpdateSceneObjects(float deltaTime)
{
    for (auto obj : m_Objects)
    {
        obj.second->OnUpdate(deltaTime);
    }
}

void MultiGpuGame::DrawSceneObjects(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    for (auto obj : m_Objects)
    {
        obj.second->OnRender(commandList, viewProjMatrix);
    }
}

void MultiGpuGame::UnloadContent()
{
    if (m_SerializeSceneOnExit)
        SaveSceneToFile();
}

void MultiGpuGame::Destroy()
{
    super::Destroy();

    m_DebugSystem.Destroy();
    m_CascadedShadowMap.Destroy();

    m_CATR.Destroy();

    m_ParticlePipeline.Destroy();
    m_ParticleComputePipeline.Destroy();
    m_SimplePipeline.Destroy();
    m_ShadowMapPipeline.Destroy();
    m_GeometryPassPipeline.Destroy();
    m_SecondDevice_SSRPipeline.Destroy();
    m_MergingPipeline.Destroy();
    m_LightPassPipeline.Destroy();

    ShaderResources::Destroy();
    DescriptorHeaps::DestroyAll();

    m_PrimaryDevice.Reset();
    m_PrimaryDevice = nullptr;

    m_SecondDevice.Reset();
    m_SecondDevice = nullptr;
}

MultiGpuGame::~MultiGpuGame()
{

}

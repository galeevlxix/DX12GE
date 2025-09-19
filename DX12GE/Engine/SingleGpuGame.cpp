#include "SingleGpuGame.h"
#include "Graphics/ResourceStorage.h"

#include <sstream>
#include <string>
#include <chrono>
#include <fstream>

SingleGpuGame::SingleGpuGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))) { }

SingleGpuGame::~SingleGpuGame()
{
    m_KatamariScene.OnExit();
    executor->Exit();

    Destroy();
    DescriptorHeaps::GetDSVHeap(GraphicAdapterPrimary)->Release();
    DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary)->Release();
    DescriptorHeaps::GetDSVHeap(GraphicAdapterSecond)->Release();
    DescriptorHeaps::GetCBVHeap(GraphicAdapterSecond)->Release();

    Application::Get().GetPrimaryDevice()->Release();
    Application::Get().GetSecondDevice()->Release();
}

bool SingleGpuGame::Initialize()
{
    m_Device = Application::Get().GetPrimaryDevice();
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    // INITIALIZE
    ShaderResources::Create();
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

    // 3D SCENE
    m_CascadedShadowMap.Create();

    m_GBuffer.Init(m_Device, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight());

    m_SSRBuffer = std::make_shared<TextureBuffer>();
    m_SSRBuffer->SetName(L"SSRResult");
    m_SSRBuffer->Init(m_Device, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    m_LightPassBuffer = std::make_shared<TextureBuffer>();
    m_LightPassBuffer->SetName(L"LightPass");
    m_LightPassBuffer->Init(m_Device, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    m_ParticleSystem.OnLoad(commandList);
    m_KatamariScene.OnLoad(commandList);
    m_Lights.Init(&m_KatamariScene);
    m_Camera.OnLoad(&(m_KatamariScene.player));
    m_Camera.Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());
    m_DebugSystem.Initialize(&m_Camera, m_Device);
    CurrentPass::Set(CurrentPass::None);

    ShaderResources::GetSSRCB()->MaxDistance = 32.0;
    ShaderResources::GetSSRCB()->RayStep = 0.03;
    ShaderResources::GetSSRCB()->Thickness = 0.0275;

    // DRAW THE CUBE
    m_DebugSystem.DrawPoint(boxPosition, 2);

    BoundingBox box(boxPosition + boxSize * 0.5, boxSize * 0.5);
    m_DebugSystem.DrawBoundingBox(box);

    m_DebugSystem.Update(commandList);

    ShaderResources::GetParticleComputeCB()->BoxPosition = boxPosition;
    ShaderResources::GetParticleComputeCB()->BoxSize = Vector3(boxSize);

    // CREATE TEXTURE3D
    tex3d.Load(commandList, boxSize.x, boxSize.y, boxSize.z);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_DepthBuffer = std::make_shared<DepthBuffer>();
    m_DepthBuffer->Init(GraphicAdapterPrimary);
    m_DepthBuffer->ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    executor = new CommandExecutor(&m_KatamariScene);

    m_Initialized = true;
    return true;
}

bool SingleGpuGame::LoadContent()
{
    return true;

}

void SingleGpuGame::OnUpdate(UpdateEventArgs& e)
{
    if (!m_Initialized) return;
    super::OnUpdate(e);
    m_Camera.OnUpdate(e.ElapsedTime);
    m_KatamariScene.OnUpdate(e.ElapsedTime);
    m_Lights.OnUpdate(e.ElapsedTime);
    ShaderResources::GetWorldCB()->LightProps.CameraPos = m_Camera.Position;
    ShaderResources::GetSSRCB()->ViewProjection = m_Camera.GetViewProjMatrix();
    ShaderResources::GetSSRCB()->CameraPos = m_Camera.Position;
    m_ParticleSystem.OnUpdate(e.ElapsedTime, stopParticles, m_Camera.GetViewProjMatrix(), m_Camera.Position);
    RefreshTitle(e);
    m_CascadedShadowMap.Update(m_Camera.Position, ShaderResources::GetWorldCB()->DirLight.Direction);
    executor->Update();
}

void SingleGpuGame::DrawSceneToShadowMaps(ComPtr<ID3D12GraphicsCommandList2> commandList)
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

        m_KatamariScene.OnRender(commandList, m_CascadedShadowMap.GetShadowViewProj(i));

        TransitionResource(commandList, shadowMap->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
}

void SingleGpuGame::DrawSceneToGBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Geometry);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_GBuffer.SetToWriteAndClear(commandList);
    m_DepthBuffer->ClearDepth(commandList);
    m_GBuffer.BindRenderTargets(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_GeometryPassPipeline.Set(commandList);
    
    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    m_KatamariScene.OnRender(commandList, m_Camera.GetViewProjMatrix());

    m_GBuffer.SetToRead(commandList);
}
 
void SingleGpuGame::LightPassRender(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Lighting);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_LightPassBuffer->SetToWriteAndClear(commandList);
    m_LightPassBuffer->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_LightPassPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    ShaderResources::SetGraphicsShadowCB(commandList, 1);
    SetGraphicsDynamicStructuredBuffer(commandList, 2, m_Lights.m_PointLights);
    SetGraphicsDynamicStructuredBuffer(commandList, 3, m_Lights.m_SpotLights);
    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(4, commandList);

    m_GBuffer.SetGraphicsRootDescriptorTable(8, GBuffer::POSITION, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(9, GBuffer::NORMAL, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(10, GBuffer::DIFFUSE, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(11, GBuffer::EMISSIVE, commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    DrawDebugObjects(commandList);
    DrawParticlesForward(commandList);

    m_LightPassBuffer->SetToRead(commandList);
}

void SingleGpuGame::DrawSSR(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::SSR);

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

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    m_SSRBuffer->SetToRead(commandList);
}

void SingleGpuGame::MergeResults(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Merging);

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

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void SingleGpuGame::DrawParticlesForward(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::TransparentParticles);

    m_ParticleComputePipeline.SetUpdatePSO(commandList);
    tex3d.Render(commandList);
    m_ParticleSystem.OnUpdateComputeRender(commandList);

    m_ParticleComputePipeline.SetSortPSO(commandList);
    m_ParticleSystem.OnSortComputeRender(commandList);

    m_ParticlePipeline.Set(commandList);
    ShaderResources::SetGraphicsWorldCB(commandList, 2);
    m_ParticleSystem.OnRender(commandList);
}

void SingleGpuGame::DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Debug);

    m_SimplePipeline.Set(commandList);
    m_DebugSystem.Draw(commandList);
}

void SingleGpuGame::OnRender(RenderEventArgs& e)
{
    if (!m_Initialized) return;
    super::OnRender(e);

    TestTime(e.ElapsedTime);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();
    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();

    DrawSceneToShadowMaps(commandList);
    DrawSceneToGBuffer(commandList);
    LightPassRender(commandList);
    //DrawSSR(commandList);
    MergeResults(commandList);

    m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
    currentBackBufferIndex = m_pWindow->Present();
    commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);

    CurrentPass::Set(CurrentPass::None);
}

void SingleGpuGame::TestTime(float elapsedTime)
{
    if (!IsTesting) return;

    if (m_Camera.IsTesting())
    {
        m_ElapsedTimeArray.push_back(elapsedTime);
    }
    else
    {
        IsTesting = false;
        ofstream out(m_TestTimeOutputFile);
        for (size_t i = 0; i < m_ElapsedTimeArray.size(); i++)
        {
            out << m_ElapsedTimeArray[i] << endl;
        }
        m_ElapsedTimeArray.clear();
        out.close();
    }
}

void SingleGpuGame::OnKeyPressed(KeyEventArgs& e)
{
    super::OnKeyPressed(e);
    
    m_Camera.OnKeyPressed(e);

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
        shouldAddDebugObjects = true;
        break;
    case KeyCode::P:
        stopParticles = !stopParticles;
        break;    
    case KeyCode::T:
        m_Camera.StartTest();
        IsTesting = true;
        break;    
    case KeyCode::R:
        auto commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto commandList = commandQueue->GetCommandList();
        m_ParticleSystem.SpawnParticleGroup(commandList, boxPosition + boxSize * 0.5, 7, 1000);
        uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
        commandQueue->WaitForFenceValue(fenceValue);
        break;
    }    
}

void SingleGpuGame::OnKeyReleased(KeyEventArgs& e)
{
    m_Camera.OnKeyReleased(e);
}

void SingleGpuGame::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_Camera.OnMouseWheel(e);
}

void SingleGpuGame::OnMouseMoved(MouseMotionEventArgs& e)
{
    m_Camera.OnMouseMoved(e);
}

void SingleGpuGame::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    m_Camera.OnMouseButtonPressed(e);
}

void SingleGpuGame::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    m_Camera.OnMouseButtonReleased(e);
}

void SingleGpuGame::OnResize(ResizeEventArgs& e)
{
    if (e.Width == GetClientWidth() && e.Height == GetClientHeight()) return;
    
    super::OnResize(e);
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));
    m_DepthBuffer->ResizeDepthBuffer(e.Width, e.Height);
    m_GBuffer.Resize(e.Width, e.Height);
    m_LightPassBuffer->Resize(e.Width, e.Height);
    m_SSRBuffer->Resize(e.Width, e.Height);
    m_Camera.Ratio = static_cast<float>(e.Width) / static_cast<float>(e.Height);
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
            rStr(m_Camera.Position.m128_f32[0], 1) + L"; " +
            rStr(m_Camera.Position.m128_f32[1], 1) + L"; " +
            rStr(m_Camera.Position.m128_f32[2], 1);
        cPos = Align(cPos, 21);

        std::wstring cTar = L" | Tar " +
            rStr(m_Camera.Target.m128_f32[0], 1) + L"; " +
            rStr(m_Camera.Target.m128_f32[1], 1) + L"; " +
            rStr(m_Camera.Target.m128_f32[2], 1);
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

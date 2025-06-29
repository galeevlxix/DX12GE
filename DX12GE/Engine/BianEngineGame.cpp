#include "BianEngineGame.h"
#include "Graphics/ResourceStorage.h"

#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>

BianEngineGame::BianEngineGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))) { }

BianEngineGame::~BianEngineGame()
{
    katamariScene.OnExit();

    Destroy();
    DescriptorHeaps::GetDSVHeap()->Release();
    DescriptorHeaps::GetCBVHeap()->Release();

    Application::Get().GetPrimaryDevice()->Release();
    //ResourceStorage::Unload();
    // todo: ������� ��� comptr �� ����������� �������
}

bool BianEngineGame::LoadContent()
{
    ComPtr<ID3D12Device2> primaryDevice = Application::Get().GetPrimaryDevice();
    shared_ptr<CommandQueue> primaryCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = primaryCommandQueue->GetCommandList();

    // INITIALIZE
    ShaderResources::Create();
    DescriptorHeaps::OnInit(primaryDevice);

    // PIPELINES
    m_GeometryPassPipeline.Initialize(primaryDevice);
    m_LightPassPipeline.Initialize(primaryDevice);
    m_ParticlePipeline.Initialize(primaryDevice);
    m_ParticleComputePipeline.Initialize(primaryDevice);
    m_SimplePipeline.Initialize(primaryDevice);
    m_ShadowMapPipeline.Initialize(primaryDevice);
    m_SSRPipeline.Initialize(primaryDevice);
    m_MergingPipeline.Initialize(primaryDevice);

    // 3D SCENE
    m_CascadedShadowMap.Create();
    m_GBuffer.Init(primaryDevice, GetClientWidth(), GetClientHeight());
    particles.OnLoad(commandList);
    particles.SpawnParticleGroup(commandList, boxPosition + boxSize * 0.5, 7, 1000);
    katamariScene.OnLoad(commandList);
    lights.Init(&(katamariScene.player));
    m_Camera.OnLoad(&(katamariScene.player));
    m_Camera.Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());
    debug.Initialize(&m_Camera, primaryDevice);
    CurrentPass::Set(CurrentPass::None);

    SSRResult.Init(primaryDevice, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
    LightPassResult.Init(primaryDevice, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    ShaderResources::GetSSRCB()->RayStep = 0.025;
    //ShaderResources::GetSSRCB()->MaxSteps = 2048;
    ShaderResources::GetSSRCB()->MaxDistance = 32.0;
    ShaderResources::GetSSRCB()->Thickness = 0.02;

    // distance 8 -> 32
    // ray step 0.025 -> 0.3
    // thickness 0.02 -> 0.25

    // DRAW THE CUBE
    debug.DrawPoint(boxPosition, 2);

    BoundingBox box(boxPosition + boxSize * 0.5, boxSize * 0.5);
    debug.DrawBoundingBox(box);
    debug.Update(commandList);

    ShaderResources::GetParticleComputeCB()->BoxPosition = boxPosition;
    ShaderResources::GetParticleComputeCB()->BoxSize = Vector3(boxSize);

    // CREATE TEXTURE3D
    tex3d.Load(commandList, boxSize.x, boxSize.y, boxSize.z);

    uint64_t fenceValue = primaryCommandQueue->ExecuteCommandList(commandList);
    primaryCommandQueue->WaitForFenceValue(fenceValue);

    m_DepthBuffer.Init();
    m_DepthBuffer.ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    return true;
}

void BianEngineGame::OnUpdate(UpdateEventArgs& e)
{
    super::OnUpdate(e);
    m_Camera.OnUpdate(e.ElapsedTime);
    katamariScene.OnUpdate(e.ElapsedTime);
    lights.OnUpdate(e.ElapsedTime);
    ShaderResources::GetWorldCB()->LightProps.CameraPos = m_Camera.Position;
    ShaderResources::GetSSRCB()->ViewProjection = m_Camera.GetViewProjMatrix();
    ShaderResources::GetSSRCB()->CameraPos = m_Camera.Position;
    particles.OnUpdate(e.ElapsedTime, stopParticles, m_Camera.GetViewProjMatrix(), m_Camera.Position);
    RefreshTitle(e);
    m_CascadedShadowMap.Update(m_Camera.Position, ShaderResources::GetWorldCB()->DirLight.Direction);
}

void BianEngineGame::DrawSceneToShadowMaps()
{
    CurrentPass::Set(CurrentPass::Shadow);

    for (int i = 0; i < CASCADES_COUNT; i++)
    {
        shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

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

        katamariScene.OnRender(commandList, m_CascadedShadowMap.GetShadowViewProj(i));

        TransitionResource(commandList, shadowMap->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);

        uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
        commandQueue->WaitForFenceValue(fenceValue);
    }
}

void BianEngineGame::DrawSceneToGBuffer()
{
    CurrentPass::Set(CurrentPass::Geometry);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer.dsvCpuHandleIndex);

    m_GBuffer.SetToWriteAndClear(commandList);
    m_DepthBuffer.ClearDepth(commandList);
    m_GBuffer.BindRenderTargets(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_GeometryPassPipeline.Set(commandList);
    
    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap().GetAddressOf());

    katamariScene.OnRender(commandList, m_Camera.GetViewProjMatrix());

    m_GBuffer.SetToRead(commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}
 
void BianEngineGame::LightPassRender()
{
    CurrentPass::Set(CurrentPass::Lighting);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();
    
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer.dsvCpuHandleIndex);

    LightPassResult.SetToWriteAndClear(commandList);
    LightPassResult.BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_LightPassPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap().GetAddressOf());

    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    ShaderResources::SetGraphicsShadowCB(commandList, 1);
    SetGraphicsDynamicStructuredBuffer(commandList, 2, lights.m_PointLights);
    SetGraphicsDynamicStructuredBuffer(commandList, 3, lights.m_SpotLights);
    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(4, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(8, GBuffer::POSITION, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(9, GBuffer::NORMAL, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(10, GBuffer::DIFFUSE, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(11, GBuffer::EMISSIVE, commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    LightPassResult.SetToRead(commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void BianEngineGame::DrawSSR()
{
    CurrentPass::Set(CurrentPass::SSR);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer.dsvCpuHandleIndex);
    SSRResult.SetToWriteAndClear(commandList);
    SSRResult.BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_SSRPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap().GetAddressOf());

    ShaderResources::SetSSRCB(commandList, 0);
    m_GBuffer.SetGraphicsRootDescriptorTable(1, GBuffer::POSITION, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(2, GBuffer::NORMAL, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTable(3, GBuffer::ORM, commandList);
    commandList->SetGraphicsRootDescriptorTable(4, LightPassResult.SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    SSRResult.SetToRead(commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void BianEngineGame::MergeResults()
{
    CurrentPass::Set(CurrentPass::Merging);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    ComPtr<ID3D12Resource> backBuffer = m_pWindow->GetCurrentBackBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_pWindow->GetCurrentRenderTargetView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer.dsvCpuHandleIndex);
    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_MergingPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap().GetAddressOf());

    commandList->SetGraphicsRootDescriptorTable(0, LightPassResult.SrvGPU());
    commandList->SetGraphicsRootDescriptorTable(1, SSRResult.SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    DrawDebugObjects(commandList);
    DrawParticlesForward(commandList);

    // Present
    {
        TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
        currentBackBufferIndex = m_pWindow->Present();
        commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
    }
}

void BianEngineGame::DrawParticlesForward(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::TransparentParticles);

    m_ParticleComputePipeline.SetUpdatePSO(commandList);
    tex3d.Render(commandList);
    particles.OnUpdateComputeRender(commandList);

    m_ParticleComputePipeline.SetSortPSO(commandList);
    particles.OnSortComputeRender(commandList);

    m_ParticlePipeline.Set(commandList);
    ShaderResources::SetGraphicsWorldCB(commandList, 2);
    particles.OnRender(commandList);
}

void BianEngineGame::DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Debug);

    m_SimplePipeline.Set(commandList);
    debug.Draw(commandList);
}

void BianEngineGame::OnRender(RenderEventArgs& e)
{
    super::OnRender(e);

    static float totalShadowDuration = 0.0f;
    static float totalGpDuration = 0.0f;
    static float totalLpDuration = 0.0f;
    static float totalSsrDuration = 0.0f;
    static float totalMergeDuration = 0.0f;

    static float timer = 0.0f;
    static float frameCounter = 0.0f;

    timer += e.ElapsedTime;
    frameCounter += 1.0f;

    {
        auto start = std::chrono::steady_clock::now();
        DrawSceneToShadowMaps();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        totalShadowDuration += duration;
    }

    {
        auto start = std::chrono::steady_clock::now();
        DrawSceneToGBuffer();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        totalGpDuration += duration;
    }

    {
        auto start = std::chrono::steady_clock::now();
        LightPassRender();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        totalLpDuration += duration;
    }
    
    {
        auto start = std::chrono::steady_clock::now();

        if (drawSSR)
            DrawSSR();
        else if (resizeSSR)
        {
            SSRResult.Init(Application::Get().GetPrimaryDevice(), GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
            resizeSSR = false;
        }

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        totalSsrDuration += duration;
    }

    {
        auto start = std::chrono::steady_clock::now();
        MergeResults();
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        totalMergeDuration += duration;
    }

    if (timer >= 1.0f)
    {
        shadowTime = totalShadowDuration / frameCounter;
        gpTime = totalGpDuration / frameCounter;
        lpTime = totalLpDuration / frameCounter;
        ssrTime = totalSsrDuration / frameCounter;
        mergeTime = totalMergeDuration / frameCounter;

        totalShadowDuration = 0.0f;
        totalGpDuration = 0.0f;
        totalLpDuration = 0.0f;
        totalSsrDuration = 0.0f;
        totalMergeDuration = 0.0f;
        
        timer = 0.0f;
        frameCounter = 0.0f;
    }

    CurrentPass::Set(CurrentPass::None);
}

void BianEngineGame::OnKeyPressed(KeyEventArgs& e)
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
        debug.canDraw = !debug.canDraw;
        shouldAddDebugObjects = true;
        break;
    case KeyCode::P:
        stopParticles = !stopParticles;
        break;
    case KeyCode::C:
        drawSSR = !drawSSR;
        resizeSSR = !drawSSR;
        
        break;
    case KeyCode::R:
        auto commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto commandList = commandQueue->GetCommandList();
        
        uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
        commandQueue->WaitForFenceValue(fenceValue);
        break;
    }    
}

void BianEngineGame::OnKeyReleased(KeyEventArgs& e)
{
    m_Camera.OnKeyReleased(e);
}

void BianEngineGame::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_Camera.OnMouseWheel(e);
}

void BianEngineGame::OnMouseMoved(MouseMotionEventArgs& e)
{
    m_Camera.OnMouseMoved(e);
}

void BianEngineGame::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    m_Camera.OnMouseButtonPressed(e);
}

void BianEngineGame::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    m_Camera.OnMouseButtonReleased(e);
}

void BianEngineGame::OnResize(ResizeEventArgs& e)
{
    if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
    {
        super::OnResize(e);
        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));
        m_DepthBuffer.ResizeDepthBuffer(e.Width, e.Height);
        m_GBuffer.Resize(e.Width, e.Height);
        SSRResult.Resize(e.Width, e.Height);
        LightPassResult.Resize(e.Width, e.Height);
    }

    m_Camera.Ratio = static_cast<float>(e.Width) / static_cast<float>(e.Height);
}

std::wstring rStr(double value, int signCount)
{
    std::wstringstream wss;
    wss << std::fixed << std::setprecision(signCount) << value;
    return wss.str();
}

std::wstring Align(std::wstring s, int cnt)
{
    std::wstring out = s; 
    while (out.length() < cnt) out += L" ";
    return out;
}

void BianEngineGame::RefreshTitle(UpdateEventArgs& e)
{
    static unsigned long frameCounter = 0;
    static double timer = 0.0;

    static const std::wstring winName = L"BianGame";

    if (timer >= 1.0)
    {
        std::wstring fps = L"Fps " + std::to_wstring(frameCounter);
        fps = Align(fps, 10);

        std::wstring shadow = L"shadow " + std::to_wstring(shadowTime);
        shadow = Align(shadow, 10);

        std::wstring geom = L"geom " + std::to_wstring(gpTime);
        geom = Align(geom, 10);

        std::wstring light = L"light " + std::to_wstring(lpTime);
        light = Align(light, 10);

        std::wstring ssr = L"ssr " + std::to_wstring(ssrTime);
        ssr = Align(ssr, 10);

        std::wstring merge = L"merge " + std::to_wstring(mergeTime);
        merge = Align(merge, 10);

        std::wstring cPos = L"Pos " +
            rStr(m_Camera.Position.m128_f32[0], 1) + L"; " +
            rStr(m_Camera.Position.m128_f32[1], 1) + L"; " +
            rStr(m_Camera.Position.m128_f32[2], 1);
        cPos = Align(cPos, 21);

        std::wstring cTar = L"Tar " +
            rStr(m_Camera.Target.m128_f32[0], 1) + L"; " +
            rStr(m_Camera.Target.m128_f32[1], 1) + L"; " +
            rStr(m_Camera.Target.m128_f32[2], 1);
        cTar = Align(cTar, 21);

        m_pWindow->UpdateWindowText(winName + L" | " + fps + L" | " + shadow + L" | " + geom + L" | " + light + L" | " + ssr + L" | " + merge + +L" | " + cPos);

        timer = 0.0;
        frameCounter = 0;
    }
    else
    {
        frameCounter++;
        timer += e.ElapsedTime;
    }
}

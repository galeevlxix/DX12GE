#include "BianGame.h"

#include <sstream>
#include <iomanip>
#include <string>

#include "../Engine/Application.h"
#include "../Engine/CommandQueue.h"
#include "../Engine/DescriptorHeaps.h"
#include "../Engine/ShaderResources.h"

BianGame::BianGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))) { }

BianGame::~BianGame()
{
    Destroy();
    DescriptorHeaps::GetDSVHeap()->Release();
    DescriptorHeaps::GetCBVHeap()->Release();

    Application::Get().GetDevice()->Release();
    // todo: ������� ��� comptr �� ����������� �������
}


bool BianGame::LoadContent()
{
    ComPtr<ID3D12Device2> device = Application::Get().GetDevice();
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    // INITIALIZE
    ShaderResources::Create();
    DescriptorHeaps::OnInit(device);

    // PIPELINES
    m_GeometryPassPipeline.Initialize(device);
    m_LightPassPipeline.Initialize(device);
    m_ParticlePipeline.Initialize(device);
    m_ParticleComputePipeline.Initialize(device);
    m_SimplePipeline.Initialize(device);
    m_ShadowMapPipeline.Initialize(device);

    // 3D SCENE
    m_CascadedShadowMap.Create();
    m_GBuffer.Init(device, GetClientWidth(), GetClientHeight());
    particles.OnLoad(commandList);
    katamariScene.OnLoad(commandList);
    lights.Init(&(katamariScene.player));
    m_Camera.OnLoad(&(katamariScene.player));
    m_Camera.Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());
    debug.Initialize(&m_Camera, device);

    // DRAW THE CUBE
    debug.DrawPoint(boxPosition, 2);

    BoundingBox box(boxPosition + boxSize * 0.5, boxSize * 0.5);
    debug.DrawBoundingBox(box);
    debug.Update(commandList);

    ShaderResources::GetParticleComputeCB()->BoxPosition = boxPosition;
    ShaderResources::GetParticleComputeCB()->BoxSize = Vector3(boxSize);

    // CREATE TEXTURE3D
    tex3d.Load(commandList, boxSize.x, boxSize.y, boxSize.z);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_DepthBuffer.Init();
    m_DepthBuffer.ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    return true;
}

void BianGame::DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    m_SimplePipeline.Set(commandList);
    debug.Draw(commandList);
}

void BianGame::OnUpdate(UpdateEventArgs& e)
{
    super::OnUpdate(e);
    m_Camera.OnUpdate(e.ElapsedTime);
    katamariScene.OnUpdate(e.ElapsedTime);
    lights.OnUpdate(e.ElapsedTime);
    ShaderResources::GetWorldCB()->LightProps.CameraPos = m_Camera.Position;
    ShaderResources::GetWorldCB()->ViewProjection = m_Camera.GetViewProjMatrix();
    particles.OnUpdate(e.ElapsedTime, stopParticles, m_Camera.GetViewProjMatrix(), m_Camera.Position);
    RefreshTitle(e);
    m_CascadedShadowMap.Update(m_Camera.Position, ShaderResources::GetWorldCB()->DirLight.Direction);
}

void BianGame::DrawSceneToShadowMaps()
{
    BaseObject::SetShadowPass(true);

    for (int i = 0; i < CASCADES_COUNT; i++)
    {
        shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
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

    BaseObject::SetShadowPass(false);
}

void BianGame::DrawSceneToGBuffer()
{
    BaseObject::SetGeometryPass(true);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
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

    BaseObject::SetGeometryPass(false);
}
 
void BianGame::LightPassRender(RenderEventArgs& e)
{
    BaseObject::SetLightPass(true);
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
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

    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap().GetAddressOf());

    m_LightPassPipeline.Set(commandList);
    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    ShaderResources::SetGraphicsShadowCB(commandList, 1);
    SetGraphicsDynamicStructuredBuffer(commandList, 2, lights.m_PointLights);
    SetGraphicsDynamicStructuredBuffer(commandList, 3, lights.m_SpotLights);
    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(4, commandList);
    m_GBuffer.SetGraphicsRootDescriptorTables(8, commandList);
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
    BaseObject::SetLightPass(false);
}

void BianGame::DrawParticlesForward(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    m_ParticleComputePipeline.SetUpdatePSO(commandList);
    tex3d.Render(commandList);
    particles.OnUpdateComputeRender(commandList);

    m_ParticleComputePipeline.SetSortPSO(commandList);
    particles.OnSortComputeRender(commandList);

    m_ParticlePipeline.Set(commandList);
    ShaderResources::SetGraphicsWorldCB(commandList, 2);
    particles.OnRender(commandList);
}

void BianGame::OnRender(RenderEventArgs& e)
{
    super::OnRender(e);
    DrawSceneToShadowMaps();
    DrawSceneToGBuffer();
    LightPassRender(e);
}

void BianGame::OnKeyPressed(KeyEventArgs& e)
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
    case KeyCode::Z:
        BaseObject::DebugMatrices();
        break;
    case KeyCode::P:
        stopParticles = !stopParticles;
        break;
    case KeyCode::R:
        auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto commandList = commandQueue->GetCommandList();
        particles.SpawnParticleGroup(commandList, boxPosition + boxSize * 0.5, 7, 1000);
        uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
        commandQueue->WaitForFenceValue(fenceValue);
        break;
    }    
}

void BianGame::OnKeyReleased(KeyEventArgs& e)
{
    m_Camera.OnKeyReleased(e);
}

void BianGame::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_Camera.OnMouseWheel(e);
}

void BianGame::OnMouseMoved(MouseMotionEventArgs& e)
{
    m_Camera.OnMouseMoved(e);
}

void BianGame::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    m_Camera.OnMouseButtonPressed(e);
}

void BianGame::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    m_Camera.OnMouseButtonReleased(e);
}

void BianGame::OnResize(ResizeEventArgs& e)
{
    if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
    {
        super::OnResize(e);
        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));
        m_DepthBuffer.ResizeDepthBuffer(e.Width, e.Height);
        m_GBuffer.Resize(GetClientWidth(), GetClientHeight());
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

void BianGame::RefreshTitle(UpdateEventArgs& e)
{
    static unsigned long frameCounter = 0;
    static double timer = 0.0;

    static const std::wstring winName = L"BianGame";

    if (timer >= 1.0)
    {
        std::wstring fps = L"Fps " + std::to_wstring(frameCounter);
        fps = Align(fps, 10);

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

        m_pWindow->UpdateWindowText(winName + L" | " + fps + L" | " + cPos + L" | " + cTar + L" | ");

        timer = 0.0;
        frameCounter = 0;
    }
    else
    {
        frameCounter++;
        timer += e.ElapsedTime;
    }
}

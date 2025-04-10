#include "BianGame.h"

#include "../Engine/Application.h"
#include "../Engine/CommandQueue.h"
#include "../Engine/DescriptorHeaps.h"

#include "../Engine/ShaderResources.h"

BianGame::BianGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
{
}

BianGame::~BianGame()
{
    DescriptorHeaps::GetDSVHeap()->Release();
    DescriptorHeaps::GetCBVHeap()->Release();
    //comptr на статический объект

    /*
    for (int i = 0; i < CASCADES_COUNT; i++)
    {
        m_CascadedShadowMap.GetShadowMap(i)->Resource()->Release();
    }
    */
}

bool BianGame::LoadContent()
{
    ComPtr<ID3D12Device2> device = Application::Get().GetDevice();
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    ShaderResourceBuffers::Create();
    DescriptorHeaps::OnInit(device);

    // 3D SCENE
    katamari.OnLoad(commandList);
    lights.Init(&(katamari.player));
    m_Camera.OnLoad(&(katamari.player));
    m_Camera.Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());

    // SHADOWS
    m_CascadedShadowMap.Create();

    // PIPELINE STATES
    m_Pipeline.Initialize(device);
    m_ShadowMapPipeline.Initialize(device);
    debug.Initialize(&m_Camera, device);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_DepthBuffer.ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    return true;
}

void BianGame::OnUpdate(UpdateEventArgs& e)
{
    super::OnUpdate(e);

    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->GetCommandList();

    m_Camera.OnUpdate(e.ElapsedTime);
    katamari.OnUpdate(e.ElapsedTime);

    lights.OnUpdate(e.ElapsedTime);
    ShaderResourceBuffers::GetWorldCB()->LightProps.CameraPos = m_Camera.Position;

    static float counter = 0;
    if (counter >= 2 * PI) counter = 0;
    ShaderResourceBuffers::GetWorldCB()->DirLight.Direction = Vector4(cos(counter), -0.5, sin(counter), 1);
    counter += PI / 4 * e.ElapsedTime;

    m_CascadedShadowMap.Update(m_Camera.Position, ShaderResourceBuffers::GetWorldCB()->DirLight.Direction);
}

// Transition a resource
void BianGame::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);
    commandList->ResourceBarrier(1, &barrier);
}

// Clear a render target view
void BianGame::ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void BianGame::AddDebugObjects()
{
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    static Vector3 prevPos(0.0f, 2.0f, 0.0f);

    Vector3 playerPosition = katamari.player.prince.Position + Vector3(0.0f, 2.0f, 0.0f);

    debug.Clear();
    debug.DrawPoint(playerPosition, 1.0f);
    debug.DrawLine(prevPos, playerPosition, Color(1.0f, 1.0f, 0.0f));
    debug.Update(commandList);
    prevPos = playerPosition;

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

template<typename T>
void BianGame::SetGraphicsDynamicStructuredBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, const vector<T>& bufferData)
{
    size_t bufferSize = bufferData.size() * sizeof(T);
    size_t size = sizeof(T);
    auto heapAllocation = ShaderResourceBuffers::GetUploadBuffer()->Allocate(bufferSize, size);
    memcpy(heapAllocation.CPU, bufferData.data(), bufferSize);
    commandList->SetGraphicsRootShaderResourceView(slot, heapAllocation.GPU);
}

template<typename T>
void BianGame::SetGraphicsConstants(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, const T& bufferData)
{
    UINT size = sizeof(T);
    commandList->SetGraphicsRoot32BitConstants(slot, size / 4, &bufferData, 0);
}

void BianGame::DrawSceneToShadowMap()
{
    for (int i = 0; i < CASCADES_COUNT; i++)
    {
        shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

        ShadowMap* shadowMap = m_CascadedShadowMap.GetShadowMap(i);

        D3D12_VIEWPORT viewport = shadowMap->Viewport();
        D3D12_RECT rect = shadowMap->ScissorRect();

        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &rect);
        TransitionResource(commandList, shadowMap->Resource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

        m_ShadowMapPipeline.Set(commandList);

        commandList->ClearDepthStencilView(shadowMap->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        auto dsv = shadowMap->Dsv();
        commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

        katamari.OnRender(commandList, m_CascadedShadowMap.GetShadowViewProj(i), true);

        TransitionResource(commandList, shadowMap->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);

        uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
        commandQueue->WaitForFenceValue(fenceValue);
    }
}

void BianGame::OnRender(RenderEventArgs& e)
{
    super::OnRender(e);

    if (shouldAddDebugObjects)
    {
        AddDebugObjects();
        shouldAddDebugObjects = false;
    }

    DrawSceneToShadowMap();

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    auto backBuffer = m_pWindow->GetCurrentBackBuffer();
    auto rtv = m_pWindow->GetCurrentRenderTargetView();
    auto dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_DepthBuffer.dsvCpuHandleIndex);

    // Clear the render targets.
    {
        TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        FLOAT clearColor[] = { 0.8f, 0.5f, 0.5f, 1.0f };

        ClearRTV(commandList, rtv, clearColor);
        m_DepthBuffer.ClearDepth(commandList);
    }
    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_Pipeline.Set(commandList);

    ShaderResourceBuffers::SetGraphicsWorldCB(commandList, 1);

    SetGraphicsDynamicStructuredBuffer(commandList, 2, lights.m_PointLights);
    SetGraphicsDynamicStructuredBuffer(commandList, 3, lights.m_SpotLights);

    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap().GetAddressOf());

    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(5, commandList);

    katamari.OnRender(commandList, m_Camera.GetViewProjMatrix());

    //debug.Draw(commandList);

    // Present
    {
        TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
        currentBackBufferIndex = m_pWindow->Present();
        commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
    }
}

void BianGame::OnKeyPressed(KeyEventArgs& e)
{
    super::OnKeyPressed(e);
    
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->GetCommandList();

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
        shouldAddDebugObjects = true;
        break;
    case KeyCode::Z:
        BaseObject::DebugMatrices();
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
    }

    m_Camera.Ratio = static_cast<float>(e.Width) / static_cast<float>(e.Height);
}

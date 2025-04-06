#include "BianGame.h"

#include "../Engine/Application.h"
#include "../Engine/CommandQueue.h"
#include "../Engine/DescriptorHeaps.h"

BianGame::BianGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
{
    m_UploadBuffer = make_unique<UploadBuffer>();
}

BianGame::~BianGame()
{
    DescriptorHeaps::GetDSVHeap()->Release();
    DescriptorHeaps::GetCBVHeap()->Release();
}

bool BianGame::LoadContent()
{
    ComPtr<ID3D12Device2> device = Application::Get().GetDevice();
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    DescriptorHeaps::OnInit(device);

    // SHADOWS
    m_ShadowMap = make_unique<ShadowMap>(device, 2048, 2048);
    m_SceneBounds.Center = Vector3(0, 0, 0);
    //float a = 75;
    float a = 75;
    m_SceneBounds.Radius = sqrtf(a * a * 2);

    // 3D SCENE
    katamari.OnLoad(commandList);
    lights.Init(&(katamari.player));
    m_Camera.OnLoad(&(katamari.player));
    m_Camera.Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());

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
    lights.m_SpecularProperties.CameraPos = m_Camera.Position;

    static float counter = 0;
    if (counter >= 2 * PI) counter = 0;

    lights.m_DirectionalLight.Direction = Vector3(cos(counter), -1, sin(counter));

    counter += PI / 4 * e.ElapsedTime;

    UpdateShadowTransform();
    UpdateShadowPassCB();
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

void BianGame::UpdateShadowTransform()
{
    lights.m_DirectionalLight.Direction.Normalize();

    mLightPosW = -2.0f * m_SceneBounds.Radius * lights.m_DirectionalLight.Direction;
    Vector3 targetPos = Vector3(m_SceneBounds.Center);
    mLightView = XMMatrixLookAtLH(mLightPosW, targetPos, Vector3(0.0f, 1.0f, 0.0f));

    // Transform bounding sphere to light space.
    Vector3 sphereCenterLS = XMVector3TransformCoord(targetPos, mLightView);

    // Ortho frustum in light space encloses scene.
    float l = sphereCenterLS.x - m_SceneBounds.Radius;
    float b = sphereCenterLS.y - m_SceneBounds.Radius;
    float n = sphereCenterLS.z - m_SceneBounds.Radius;
    float r = sphereCenterLS.x + m_SceneBounds.Radius;
    float t = sphereCenterLS.y + m_SceneBounds.Radius;
    float f = sphereCenterLS.z + m_SceneBounds.Radius;

    mLightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    mShadowTransform = XMMatrixMultiply(mLightView, mLightProj);
    
    BaseObject::SetSMMatrix(mShadowTransform);
}

void BianGame::UpdateShadowPassCB()
{
    Matrix viewProj = mLightView * mLightProj;
    XMVECTOR detView = XMMatrixDeterminant(mLightView);
    Matrix invView = XMMatrixInverse(&detView, mLightView);
    XMVECTOR detProj = XMMatrixDeterminant(mLightProj);
    Matrix invProj = XMMatrixInverse(&detProj, mLightProj);
    XMVECTOR detViewProj = XMMatrixDeterminant(viewProj);
    Matrix invViewProj = XMMatrixInverse(&detViewProj, viewProj);

    UINT w = m_ShadowMap->Width();
    UINT h = m_ShadowMap->Height();
}

template<typename T>
void BianGame::SetGraphicsDynamicStructuredBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList, uint32_t slot, const vector<T>& bufferData)
{
    size_t bufferSize = bufferData.size() * sizeof(T);
    auto heapAllocation = m_UploadBuffer->Allocate(bufferSize, sizeof(T));
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
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    auto vp = m_ShadowMap->Viewport();
    auto sr = m_ShadowMap->ScissorRect();

    commandList->RSSetViewports(1, &vp);
    commandList->RSSetScissorRects(1, &sr);
    TransitionResource(commandList, m_ShadowMap->Resource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

    m_ShadowMapPipeline.Set(commandList);

    commandList->ClearDepthStencilView(m_ShadowMap->Dsv(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    auto dsv = m_ShadowMap->Dsv();
    commandList->OMSetRenderTargets(0, nullptr, false, &dsv);

    katamari.OnRender(commandList, mShadowTransform, true);

    TransitionResource(commandList, m_ShadowMap->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
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
    auto dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

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

    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap().GetAddressOf());

    SetGraphicsConstants(commandList, 2, lights.m_AmbientLight);
    SetGraphicsConstants(commandList, 3, lights.m_DirectionalLight);
    //SetGraphicsConstants(commandList, 4, lights.m_LightProperties);
    //SetGraphicsDynamicStructuredBuffer(commandList, 5, lights.m_PointLights);
    //SetGraphicsDynamicStructuredBuffer(commandList, 6, lights.m_SpotLights);
    //SetGraphicsConstants(commandList, 7, lights.m_SpecularProperties);

    commandList->SetGraphicsRootDescriptorTable(4, DescriptorHeaps::GetGPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 0));

    //commandList->SetGraphicsRoot32BitConstants(9, sizeof(XMMATRIX) / 4, &mShadowTransform, 0);

    katamari.OnRender(commandList, m_Camera.GetViewProjMatrix());

    debug.Draw(commandList);

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
        BaseObject::DebugMatrix();
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

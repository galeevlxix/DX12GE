#include "BianGame.h"

#include "../Engine/Application.h"
#include "../Engine/CommandQueue.h"

BianGame::BianGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
{
    m_UploadBuffer = make_unique<UploadBuffer>();
}

bool BianGame::LoadContent()
{
    ComPtr<ID3D12Device2> device = Application::Get().GetDevice();
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    m_ShadowMap = make_unique<ShadowMap>(device, 2048, 2048);
    m_SceneBounds.Center = Vector3(0, 0, 0);
    float a = 20;
    m_SceneBounds.Radius = sqrtf(a * a * 2);

    katamari.OnLoad(commandList);
    lights.Init(&(katamari.player));

    m_Camera.OnLoad(&(katamari.player));
    m_Camera.Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());

    m_Pipeline.Initialize(device);
    m_ShadowMapPipeline.Initialize(device);

    debug.Init(&m_Camera, device);

    UpdateShadowTransform();
    UpdateShadowPassCB();
    debug.Update(commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_DepthBuffer.InitDSV();
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

    debug.DrawPoint(playerPosition, 1.0f);
    debug.DrawLine(prevPos, playerPosition, Color(1.0f, 1.0f, 0.0f));
    debug.Update(commandList);
    prevPos = playerPosition;

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void BianGame::UpdateShadowTransform()
{
    Vector3 lightDir = lights.m_DirectionalLight.Direction;
    mLightPosW = -2.0f * m_SceneBounds.Radius * lightDir;
    Vector3 targetPos = Vector3(m_SceneBounds.Center);
    Vector3 lightUp = Vector3(0.0f, 1.0f, 0.0f);
    mLightView = XMMatrixLookAtLH(mLightPosW, targetPos, lightUp);

    // Transform bounding sphere to light space.
    Vector3 sphereCenterLS = XMVector3TransformCoord(targetPos, mLightView);

    // Ortho frustum in light space encloses scene.
    float l = sphereCenterLS.x - m_SceneBounds.Radius;
    float b = sphereCenterLS.y - m_SceneBounds.Radius;
    float n = sphereCenterLS.z - m_SceneBounds.Radius;
    float r = sphereCenterLS.x + m_SceneBounds.Radius;
    float t = sphereCenterLS.y + m_SceneBounds.Radius;
    float f = sphereCenterLS.z + m_SceneBounds.Radius;

    mLightNearZ = n;
    mLightFarZ = f;

    mLightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

    // Transform NDC space [-1,+1]^2 to texture space [0,1]^2
    Matrix T(
        0.5f,   0.0f,   0.0f,   0.0f,
        0.0f,   -0.5f,  0.0f,   0.0f,
        0.0f,   0.0f,   1.0f,   0.0f,
        0.5f,   0.5f,   0.0f,   1.0f);

    mShadowTransform = mLightView * mLightProj;// * T;

    debug.DrawFrustrum(mLightView, mLightProj);
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

    struct PassConstants
    {
        Matrix View;
        Matrix InvView;
        Matrix Proj;
        Matrix InvProj;
        Matrix ViewProj;
        Matrix InvViewProj;
        Matrix ShadowTransform;

        Vector3 EyePosW = { 0.0f, 0.0f, 0.0f };
        float cbPerObjectPad1 = 0.0f;
        Vector2 RenderTargetSize = { 0.0f, 0.0f };
        Vector2 InvRenderTargetSize = { 0.0f, 0.0f };
        float NearZ = 0.0f;
        float FarZ = 0.0f;
    } mShadowPassCB;

    mShadowPassCB.View = mLightView;
    mShadowPassCB.InvView = invView;
    mShadowPassCB.Proj = XMMatrixTranspose(mLightProj);
    mShadowPassCB.InvProj = XMMatrixTranspose(invProj);
    mShadowPassCB.ViewProj = XMMatrixTranspose(viewProj);
    mShadowPassCB.InvViewProj = XMMatrixTranspose(invViewProj);
    mShadowPassCB.EyePosW = mLightPosW;
    mShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
    mShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
    mShadowPassCB.NearZ = mLightNearZ;
    mShadowPassCB.FarZ = mLightFarZ;
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
    auto dsv = m_DepthBuffer.DSVHeap->GetCPUDescriptorHandleForHeapStart();

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

    SetGraphicsConstants(commandList, 2, lights.m_AmbientLight);
    SetGraphicsConstants(commandList, 3, lights.m_DirectionalLight);
    SetGraphicsConstants(commandList, 4, lights.m_LightProperties);
    SetGraphicsDynamicStructuredBuffer(commandList, 5, lights.m_PointLights);
    SetGraphicsDynamicStructuredBuffer(commandList, 6, lights.m_SpotLights);
    SetGraphicsConstants(commandList, 7, lights.m_SpecularProperties);

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

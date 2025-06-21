#include "BianEngineGame.h"
#include "Graphics/ResourceStorage.h"

#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <fstream>

BianEngineGame::BianEngineGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))) { }

BianEngineGame::~BianEngineGame()
{
    katamariScene.OnExit();
    executor->Exit();

    Destroy();
    DescriptorHeaps::GetDSVHeap(GraphicAdapterPrimary)->Release();
    DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary)->Release();
    DescriptorHeaps::GetDSVHeap(GraphicAdapterSecond)->Release();
    DescriptorHeaps::GetCBVHeap(GraphicAdapterSecond)->Release();

    Application::Get().GetPrimaryDevice()->Release();
    Application::Get().GetSecondDevice()->Release();
    //ResourceStorage::Unload();
    // todo: удалить все comptr на статические объекты
}

bool BianEngineGame::LoadContent()
{
    ComPtr<ID3D12Device2> primaryDevice = Application::Get().GetPrimaryDevice();
    ComPtr<ID3D12Device2> secondDevice = Application::Get().GetSecondDevice();
    shared_ptr<CommandQueue> primaryCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = primaryCommandQueue->GetCommandList();

    // INITIALIZE
    ShaderResources::Create();
    DescriptorHeaps::OnInit(primaryDevice, GraphicAdapterPrimary);
    DescriptorHeaps::OnInit(secondDevice, GraphicAdapterSecond);

    // PIPELINES
    m_GeometryPassPipeline.Initialize(primaryDevice);
    m_LightPassPipeline.Initialize(primaryDevice);
    m_ParticlePipeline.Initialize(primaryDevice);
    m_ParticleComputePipeline.Initialize(primaryDevice);
    m_SimplePipeline.Initialize(primaryDevice);
    m_ShadowMapPipeline.Initialize(primaryDevice);
    m_SSRPipelinePrimaryDevice.Initialize(primaryDevice);
    m_SSRPipelineSecondDevice.Initialize(secondDevice);
    m_MergingPipeline.Initialize(primaryDevice);

    // 3D SCENE
    m_CascadedShadowMap.Create();
    
    m_PrimaryDeviceGBuffer.Init(primaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight());
    m_PrimaryDeviceSSRResult = std::make_shared<TextureBuffer>();
    m_PrimaryDeviceSSRResult->Init(primaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
    m_PrimaryDeviceLightPassResult = std::make_shared<TextureBuffer>();
    m_PrimaryDeviceLightPassResult->Init(primaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
    
    m_SecondDeviceGBuffer.Init(secondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight());
    
    m_SecondDeviceSSRResult = std::make_shared<TextureBuffer>();
    m_SecondDeviceSSRResult->Init(secondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
    
    m_SecondDeviceLightPassResult = std::make_shared<TextureBuffer>();
    m_SecondDeviceLightPassResult->Init(secondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    m_SharedMemory = std::make_unique<SharedMemory>();

    particles.OnLoad(commandList);
    
    katamariScene.OnLoad(commandList);
    lights.Init(&katamariScene);
    m_Camera.OnLoad(&(katamariScene.player));
    m_Camera.Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());
    debug.Initialize(&m_Camera, primaryDevice);
    CurrentPass::Set(CurrentPass::None);

    ShaderResources::GetSSRCB()->MaxDistance = 32.0;
    ShaderResources::GetSSRCB()->RayStep = 0.3;
    ShaderResources::GetSSRCB()->Thickness = 0.24;

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

    m_PrimaryDeviceDepthBuffer.Init(GraphicAdapterPrimary);
    m_PrimaryDeviceDepthBuffer.ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    m_SecondDeviceDepthBuffer.Init(GraphicAdapterSecond);
    m_SecondDeviceDepthBuffer.ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    executor = new CommandExecutor(&katamariScene);

    m_Initialized = true;
    return true;
}

void BianEngineGame::OnUpdate(UpdateEventArgs& e)
{
    if (!m_Initialized) return;
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
    executor->Update();
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

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_PrimaryDeviceDepthBuffer.dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_PrimaryDeviceGBuffer.SetToWriteAndClear(commandList);
    m_PrimaryDeviceDepthBuffer.ClearDepth(commandList);
    m_PrimaryDeviceGBuffer.BindRenderTargets(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_GeometryPassPipeline.Set(commandList);
    
    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    katamariScene.OnRender(commandList, m_Camera.GetViewProjMatrix());

    m_PrimaryDeviceGBuffer.SetToRead(commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}
 
void BianEngineGame::LightPassRender()
{
    CurrentPass::Set(CurrentPass::Lighting);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();
    
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_PrimaryDeviceDepthBuffer.dsvCpuHandleIndex, GraphicAdapterPrimary);

    m_PrimaryDeviceLightPassResult->SetToWriteAndClear(commandList);
    m_PrimaryDeviceLightPassResult->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_LightPassPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    ShaderResources::SetGraphicsShadowCB(commandList, 1);
    SetGraphicsDynamicStructuredBuffer(commandList, 2, lights.m_PointLights);
    SetGraphicsDynamicStructuredBuffer(commandList, 3, lights.m_SpotLights);
    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(4, commandList);
    m_PrimaryDeviceGBuffer.SetGraphicsRootDescriptorTable(8, GBuffer::POSITION, commandList);
    m_PrimaryDeviceGBuffer.SetGraphicsRootDescriptorTable(9, GBuffer::NORMAL, commandList);
    m_PrimaryDeviceGBuffer.SetGraphicsRootDescriptorTable(10, GBuffer::DIFFUSE, commandList);
    m_PrimaryDeviceGBuffer.SetGraphicsRootDescriptorTable(11, GBuffer::EMISSIVE, commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    DrawDebugObjects(commandList);
    DrawParticlesForward(commandList);

    m_PrimaryDeviceLightPassResult->SetToRead(commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void BianEngineGame::DrawSSR()
{
    CurrentPass::Set(CurrentPass::SSR);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_PrimaryDeviceDepthBuffer.dsvCpuHandleIndex, GraphicAdapterPrimary);
    m_PrimaryDeviceSSRResult->SetToWriteAndClear(commandList);
    m_PrimaryDeviceSSRResult->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_SSRPipelinePrimaryDevice.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    ShaderResources::SetSSRCB(commandList, 0, GraphicAdapterPrimary);
    m_PrimaryDeviceGBuffer.SetGraphicsRootDescriptorTable(1, GBuffer::POSITION, commandList);
    m_PrimaryDeviceGBuffer.SetGraphicsRootDescriptorTable(2, GBuffer::NORMAL, commandList);
    m_PrimaryDeviceGBuffer.SetGraphicsRootDescriptorTable(3, GBuffer::ORM, commandList);
    commandList->SetGraphicsRootDescriptorTable(4, m_PrimaryDeviceLightPassResult->SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    m_PrimaryDeviceSSRResult->SetToRead(commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void BianEngineGame::DrawSSRSecondDevice()
{
    shared_ptr<CommandQueue> secondDeviceCommandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> secondDeviceCommandList = secondDeviceCommandQueue->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_SecondDeviceDepthBuffer.dsvCpuHandleIndex, GraphicAdapterSecond);
    
    m_SecondDeviceSSRResult->SetToWriteAndClear(secondDeviceCommandList);
    m_SecondDeviceSSRResult->BindRenderTarget(secondDeviceCommandList, dsv);

    secondDeviceCommandList->RSSetViewports(1, &m_Viewport);
    secondDeviceCommandList->RSSetScissorRects(1, &m_ScissorRect);

    m_SSRPipelineSecondDevice.Set(secondDeviceCommandList);
    secondDeviceCommandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterSecond).GetAddressOf());

    ShaderResources::SetSSRCB(secondDeviceCommandList, 0, GraphicAdapterSecond);
    m_SecondDeviceGBuffer.SetGraphicsRootDescriptorTable(1, GBuffer::POSITION, secondDeviceCommandList);
    m_SecondDeviceGBuffer.SetGraphicsRootDescriptorTable(2, GBuffer::NORMAL, secondDeviceCommandList);
    m_SecondDeviceGBuffer.SetGraphicsRootDescriptorTable(3, GBuffer::ORM, secondDeviceCommandList);
    secondDeviceCommandList->SetGraphicsRootDescriptorTable(4, m_SecondDeviceLightPassResult->SrvGPU());

    secondDeviceCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    secondDeviceCommandList->DrawInstanced(3, 1, 0, 0);

    m_SecondDeviceSSRResult->SetToRead(secondDeviceCommandList);

    uint64_t fenceValue = secondDeviceCommandQueue->ExecuteCommandList(secondDeviceCommandList);
    secondDeviceCommandQueue->WaitForFenceValue(fenceValue);
}

void BianEngineGame::MergeResults()
{
    CurrentPass::Set(CurrentPass::Merging);

    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    ComPtr<ID3D12Resource> backBuffer = m_pWindow->GetCurrentBackBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_pWindow->GetCurrentRenderTargetView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_PrimaryDeviceDepthBuffer.dsvCpuHandleIndex, GraphicAdapterPrimary);
    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_MergingPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    commandList->SetGraphicsRootDescriptorTable(0, m_PrimaryDeviceLightPassResult->SrvGPU());
    commandList->SetGraphicsRootDescriptorTable(1, m_PrimaryDeviceSSRResult->SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    // Present
    {
        TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
        currentBackBufferIndex = m_pWindow->Present();
        commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
    }
}

void BianEngineGame::CopyPrimaryDeviceDataToSharedMemory()
{
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    m_SharedMemory->CopyToSharedMemory(SharedMemoryPositionBuffer, m_PrimaryDeviceGBuffer.GetBuffer(GBuffer::POSITION), commandList);
    m_SharedMemory->CopyToSharedMemory(SharedMemoryNormalBuffer, m_PrimaryDeviceGBuffer.GetBuffer(GBuffer::NORMAL), commandList);
    m_SharedMemory->CopyToSharedMemory(SharedMemoryORMBuffer, m_PrimaryDeviceGBuffer.GetBuffer(GBuffer::ORM), commandList);
    m_SharedMemory->CopyToSharedMemory(SharedMemoryLightPassBuffer, m_PrimaryDeviceLightPassResult, commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void BianEngineGame::CopySecondDeviceDataToSharedMemory()
{
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    m_SharedMemory->CopyToSharedMemory(SharedMemorySSRBuffer, m_SecondDeviceSSRResult, commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void BianEngineGame::CopySharedMemoryDataToPrimaryDevice()
{
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    m_SharedMemory->CopyFromSharedMemory(SharedMemorySSRBuffer, m_PrimaryDeviceSSRResult, commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
}

void BianEngineGame::CopySharedMemoryDataToSecondDevice()
{
    shared_ptr<CommandQueue> commandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    m_SharedMemory->CopyFromSharedMemory(SharedMemoryPositionBuffer, m_SecondDeviceGBuffer.GetBuffer(GBuffer::POSITION), commandList);
    m_SharedMemory->CopyFromSharedMemory(SharedMemoryNormalBuffer, m_SecondDeviceGBuffer.GetBuffer(GBuffer::NORMAL), commandList);
    m_SharedMemory->CopyFromSharedMemory(SharedMemoryORMBuffer, m_SecondDeviceGBuffer.GetBuffer(GBuffer::ORM), commandList);
    m_SharedMemory->CopyFromSharedMemory(SharedMemoryLightPassBuffer, m_SecondDeviceLightPassResult, commandList);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);
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
    if (!m_Initialized) return;

    TestTime("../../DX12GE/Resources/single_gpu.txt", e.ElapsedTime);

    super::OnRender(e);

    DrawSceneToShadowMaps();
    DrawSceneToGBuffer();
    LightPassRender();

    if (drawSSR)
    {
        //DrawSSR();
        CopyPrimaryDeviceDataToSharedMemory();  
        CopySharedMemoryDataToSecondDevice();
        DrawSSRSecondDevice();
        CopySecondDeviceDataToSharedMemory();
        CopySharedMemoryDataToPrimaryDevice();
    }
    else if (resizeSSR)
    {
        m_PrimaryDeviceSSRResult->Init(Application::Get().GetPrimaryDevice(), GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
        resizeSSR = false;
    }
    MergeResults();

    CurrentPass::Set(CurrentPass::None);
}

void BianEngineGame::TestTime(string outputFile, float elapsedTime)
{
    if (!IsTesting) return;

    if (m_Camera.IsTesting())
    {
        elapsed.push_back(elapsedTime);
    }
    else
    {
        IsTesting = false;
        ofstream out(outputFile);
        for (size_t i = 0; i < elapsed.size(); i++)
        {
            out << elapsed[i] << endl;
        }
        elapsed.clear();
        out.close();
    }
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
    case KeyCode::T:
        m_Camera.StartTest();
        IsTesting = true;
        break;
    case KeyCode::R:
        auto commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto commandList = commandQueue->GetCommandList();
        particles.SpawnParticleGroup(commandList, boxPosition + boxSize * 0.5, 7, 1000);
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

        m_PrimaryDeviceDepthBuffer.ResizeDepthBuffer(e.Width, e.Height);
        m_PrimaryDeviceGBuffer.Resize(e.Width, e.Height);
        m_PrimaryDeviceSSRResult->Resize(e.Width, e.Height);
        m_PrimaryDeviceLightPassResult->Resize(e.Width, e.Height);

        m_SecondDeviceDepthBuffer.ResizeDepthBuffer(e.Width, e.Height);
        m_SecondDeviceGBuffer.Resize(e.Width, e.Height);
        m_SecondDeviceSSRResult->Resize(e.Width, e.Height);
        m_SecondDeviceLightPassResult->Resize(e.Width, e.Height);
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
        std::wstring fps = L" | Fps " + std::to_wstring(frameCounter);
        fps = Align(fps, 10);

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

        m_pWindow->UpdateWindowText(winName + fps + cPos + cTar);

        timer = 0.0;
        frameCounter = 0;
    }
    else
    {
        frameCounter++;
        timer += e.ElapsedTime;
    }
}

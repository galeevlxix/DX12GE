#include "MultiGpuGame.h"
#include "Graphics/ResourceStorage.h"

#include <sstream>
#include <string>
#include <chrono>
#include <fstream>

MultiGpuGame::MultiGpuGame(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))) { }

MultiGpuGame::~MultiGpuGame()
{
    katamariScene.Exit();
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

bool MultiGpuGame::Initialize()
{
    if (!super::Initialize()) return false;

    return true;
}

bool MultiGpuGame::LoadContent()
{
    PrimaryDevice = Application::Get().GetPrimaryDevice();
    SecondDevice = Application::Get().GetSecondDevice();

    shared_ptr<CommandQueue> primaryCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    ComPtr<ID3D12GraphicsCommandList2> primaryCommandList = primaryCommandQueue->GetCommandList();

    //shared_ptr<SharedFenceCommandQueues> commandQueues = Application::Get().GetSharedCommandQueues();
    //ComPtr<ID3D12GraphicsCommandList2> primaryCommandList = commandQueues->GetCommandList(GraphicAdapterPrimary);
    //ComPtr<ID3D12GraphicsCommandList2> secondCommandList = commandQueues->GetCommandList(GraphicAdapterSecond);

    // INITIALIZE
    ShaderResources::Create();
    DescriptorHeaps::OnInit(PrimaryDevice, GraphicAdapterPrimary);
    DescriptorHeaps::OnInit(SecondDevice, GraphicAdapterSecond);

    // PIPELINES
    m_GeometryPassPipeline.Initialize(PrimaryDevice);
    m_LightPassPipeline.Initialize(PrimaryDevice);
    m_ParticlePipeline.Initialize(PrimaryDevice);
    m_ParticleComputePipeline.Initialize(PrimaryDevice);
    m_SimplePipeline.Initialize(PrimaryDevice);
    m_ShadowMapPipeline.Initialize(PrimaryDevice);
    m_SSRPipelinePrimaryDevice = std::make_shared<SSRPipeline>();
    m_SSRPipelinePrimaryDevice->Initialize(PrimaryDevice);
    m_SSRPipelineSecondDevice = std::make_shared<SSRPipeline>();
    m_SSRPipelineSecondDevice->Initialize(SecondDevice);
    m_MergingPipeline.Initialize(PrimaryDevice);

    // 3D SCENE
    m_CascadedShadowMap.Create(PrimaryDevice);
    

    CATR.PrimaryGBuffer = std::make_shared<GBuffer>();
    CATR.PrimaryGBuffer->Init(PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight());

    CATR.PrimarySSRResult = std::make_shared<TextureBuffer>();
    CATR.PrimarySSRResult->SetName(L"SSRResultPrimaryResource");
    CATR.PrimarySSRResult->Init(PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);

    CATR.PrimaryLightPassResult = std::make_shared<TextureBuffer>();
    CATR.PrimaryLightPassResult->SetName(L"LightPassPrimaryResource");
    CATR.PrimaryLightPassResult->Init(PrimaryDevice, GraphicAdapterPrimary, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
    

    CATR.SecondGBuffer = std::make_shared<GBuffer>();
    CATR.SecondGBuffer->Init(SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight());
    
    CATR.SecondSSRResult = std::make_shared<TextureBuffer>();
    CATR.SecondSSRResult->SetName(L"SSRResultSecondResource");
    CATR.SecondSSRResult->Init(SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);
    
    CATR.SecondLightPassResult = std::make_shared<TextureBuffer>();
    CATR.SecondLightPassResult->SetName(L"LightPassSecondResource");
    CATR.SecondLightPassResult->Init(SecondDevice, GraphicAdapterSecond, GetClientWidth(), GetClientHeight(), DXGI_FORMAT_R8G8B8A8_UNORM);


    CATR.PrimaryDevice = PrimaryDevice;
    CATR.SecondDevice = SecondDevice;
    CATR.Initialize();


    particles.OnLoad(primaryCommandList);
    
    katamariScene.OnLoad(primaryCommandList);
    lights.Init(&katamariScene);
    m_Camera.OnLoad(&(katamariScene.player));
    m_Camera.Ratio = static_cast<float>(GetClientWidth()) / static_cast<float>(GetClientHeight());
    debug.Initialize(&m_Camera, PrimaryDevice);
    CurrentPass::Set(CurrentPass::None);

    ShaderResources::GetSSRCB()->MaxDistance = 32.0f;
    ShaderResources::GetSSRCB()->RayStep = 0.03f;
    ShaderResources::GetSSRCB()->Thickness = 0.0275f;

    // DRAW THE CUBE
    debug.DrawPoint(boxPosition, 2.0f);

    BoundingBox box(boxPosition + boxSize * 0.5f, boxSize * 0.5f);
    debug.DrawBoundingBox(box);

    debug.Update(primaryCommandList);

    ShaderResources::GetParticleComputeCB()->BoxPosition = boxPosition;
    ShaderResources::GetParticleComputeCB()->BoxSize = Vector3(boxSize);

    // CREATE TEXTURE3D
    tex3d.Load(primaryCommandList, static_cast<int>(boxSize.x), static_cast<int>(boxSize.y), static_cast<int>(boxSize.z));

    uint64_t fenceValue = primaryCommandQueue->ExecuteCommandList(primaryCommandList);
    primaryCommandQueue->WaitForFenceValue(fenceValue);

    CATR.PrimaryDepthBuffer = std::make_shared<DepthBuffer>();
    CATR.PrimaryDepthBuffer->Init(GraphicAdapterPrimary);
    CATR.PrimaryDepthBuffer->ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    CATR.SecondDepthBuffer = std::make_shared<DepthBuffer>();
    CATR.SecondDepthBuffer->Init(GraphicAdapterSecond);
    CATR.SecondDepthBuffer->ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    executor = new CommandExecutor(&katamariScene);

    m_Initialized = true;
    return true;
}

void MultiGpuGame::UnloadContent()
{

}

void MultiGpuGame::OnUpdate(UpdateEventArgs& e)
{
    if (!m_Initialized) return;
    super::OnUpdate(e);
    float elapsedTime = static_cast<float>(e.ElapsedTime);
    m_Camera.OnUpdate(elapsedTime);
    katamariScene.OnUpdate(elapsedTime);
    lights.OnUpdate(elapsedTime);
    ShaderResources::GetWorldCB()->LightProps.CameraPos = m_Camera.Position;
    ShaderResources::GetSSRCB()->ViewProjection = m_Camera.GetViewProjMatrix();
    ShaderResources::GetSSRCB()->CameraPos = m_Camera.Position;
    particles.OnUpdate(elapsedTime, stopParticles, m_Camera.GetViewProjMatrix(), m_Camera.Position);
    RefreshTitle(e);
    m_CascadedShadowMap.Update(m_Camera.Position, ShaderResources::GetWorldCB()->DirLight.Direction);
    executor->Update();
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

        katamariScene.OnRender(commandList, m_CascadedShadowMap.GetShadowViewProj(i));

        TransitionResource(commandList, shadowMap->Resource(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
    }
}

void MultiGpuGame::DrawSceneToGBuffer(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Geometry);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, CATR.PrimaryDepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    CATR.PrimaryGBuffer->SetToWriteAndClear(commandList);
    CATR.PrimaryDepthBuffer->ClearDepth(commandList);
    CATR.PrimaryGBuffer->BindRenderTargets(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_GeometryPassPipeline.Set(commandList);
    
    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    katamariScene.OnRender(commandList, m_Camera.GetViewProjMatrix());

    CATR.PrimaryGBuffer->SetToRead(commandList);
}
 
void MultiGpuGame::LightPassRender(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Lighting);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, CATR.PrimaryDepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);

    CATR.PrimaryLightPassResult->SetToWriteAndClear(commandList);
    CATR.PrimaryLightPassResult->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_LightPassPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    ShaderResources::SetGraphicsWorldCB(commandList, 0);
    ShaderResources::SetGraphicsShadowCB(commandList, 1);
    SetGraphicsDynamicStructuredBuffer(commandList, 2, lights.m_PointLights);
    SetGraphicsDynamicStructuredBuffer(commandList, 3, lights.m_SpotLights);
    m_CascadedShadowMap.SetGraphicsRootDescriptorTables(4, commandList);
    CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(8, GBuffer::POSITION, commandList);
    CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(9, GBuffer::NORMAL, commandList);
    CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(10, GBuffer::DIFFUSE, commandList);
    CATR.PrimaryGBuffer->SetGraphicsRootDescriptorTable(11, GBuffer::EMISSIVE, commandList);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    DrawDebugObjects(commandList);
    DrawParticlesForward(commandList);

    CATR.PrimaryLightPassResult->SetToRead(commandList);
}

void MultiGpuGame::DrawSSR(
    ComPtr<ID3D12GraphicsCommandList2> commandList, 
    GraphicsAdapter graphicsAdapter, 
    std::shared_ptr<DepthBuffer> depthBuffer,
    std::shared_ptr<GBuffer> gBuffer,
    std::shared_ptr<TextureBuffer> lightPassResult,
    std::shared_ptr<TextureBuffer> ssrResult,
    std::shared_ptr<SSRPipeline> ssrPipeline)
{
    //CurrentPass::Set(CurrentPass::SSR);

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, depthBuffer->dsvCpuHandleIndex, graphicsAdapter);
    
    ssrResult->SetToWriteAndClear(commandList);
    ssrResult->BindRenderTarget(commandList, dsv);

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    ssrPipeline->Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(graphicsAdapter).GetAddressOf());

    ShaderResources::SetSSRCB(commandList, 0, graphicsAdapter);
    gBuffer->SetGraphicsRootDescriptorTable(1, GBuffer::POSITION, commandList);
    gBuffer->SetGraphicsRootDescriptorTable(2, GBuffer::NORMAL, commandList);
    gBuffer->SetGraphicsRootDescriptorTable(3, GBuffer::ORM, commandList);
    commandList->SetGraphicsRootDescriptorTable(4, lightPassResult->SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    ssrResult->SetToRead(commandList);
}

void MultiGpuGame::MergeResults(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Merging);

    ComPtr<ID3D12Resource> backBuffer = m_pWindow->GetCurrentBackBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_pWindow->GetCurrentRenderTargetView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DescriptorHeaps::GetCPUHandle(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, CATR.PrimaryDepthBuffer->dsvCpuHandleIndex, GraphicAdapterPrimary);
    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    m_MergingPipeline.Set(commandList);
    commandList->SetDescriptorHeaps(1, DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).GetAddressOf());

    commandList->SetGraphicsRootDescriptorTable(0, CATR.PrimaryLightPassResult->SrvGPU());
    commandList->SetGraphicsRootDescriptorTable(1, CATR.PrimarySSRResult->SrvGPU());

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->DrawInstanced(3, 1, 0, 0);

    TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

void MultiGpuGame::DrawParticlesForward(ComPtr<ID3D12GraphicsCommandList2> commandList)
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

void MultiGpuGame::DrawDebugObjects(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    CurrentPass::Set(CurrentPass::Debug);

    m_SimplePipeline.Set(commandList);
    debug.Draw(commandList);
}

void MultiGpuGame::DrawSingleGpu()
{
    shared_ptr<CommandQueue> primaryDeviceCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> primaryDeviceCommandList = primaryDeviceCommandQueue->GetCommandList();
    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();

    DrawSceneToShadowMaps(primaryDeviceCommandList);
    DrawSceneToGBuffer(primaryDeviceCommandList);
    LightPassRender(primaryDeviceCommandList);
    DrawSSR(
        primaryDeviceCommandList, 
        GraphicAdapterPrimary, 
        CATR.PrimaryDepthBuffer,
        CATR.PrimaryGBuffer,
        CATR.PrimaryLightPassResult,
        CATR.PrimarySSRResult,
        m_SSRPipelinePrimaryDevice);
    MergeResults(primaryDeviceCommandList);   

    {
        auto start = std::chrono::steady_clock::now();

        m_FenceValues[currentBackBufferIndex] = primaryDeviceCommandQueue->ExecuteCommandList(primaryDeviceCommandList);
        currentBackBufferIndex = m_pWindow->Present();
        primaryDeviceCommandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << duration << ": primaryDevice" << std::endl;
    }
}

void MultiGpuGame::DrawMultiGpuTest()
{
    shared_ptr<CommandQueue> primaryDeviceCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    shared_ptr<CommandQueue> secondDeviceCommandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    std::thread ssrThread([&] 
        {
            if (!m_IsFirstFrame)
            {
                {
                    auto start = std::chrono::steady_clock::now();

                    ComPtr<ID3D12GraphicsCommandList2> secondDeviceCommandList = secondDeviceCommandQueue->GetCommandList();

                    CATR.CopySharedMemoryDataToSecondDevice(secondDeviceCommandList);

                    uint64_t fenceValueSSR = secondDeviceCommandQueue->ExecuteCommandList(secondDeviceCommandList);
                    secondDeviceCommandQueue->WaitForFenceValue(fenceValueSSR);

                    auto end = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    std::cout << duration << ": CopySharedMemoryDataToSecondDevice" << std::endl;
                }

                {
                    auto start = std::chrono::steady_clock::now();

                    ComPtr<ID3D12GraphicsCommandList2> secondDeviceCommandList = secondDeviceCommandQueue->GetCommandList();
                    
                    DrawSSR(
                        secondDeviceCommandList,
                        GraphicAdapterSecond,
                        CATR.SecondDepthBuffer,
                        CATR.SecondGBuffer,
                        CATR.SecondLightPassResult,
                        CATR.SecondSSRResult,
                        m_SSRPipelineSecondDevice);

                    uint64_t fenceValueSSR = secondDeviceCommandQueue->ExecuteCommandList(secondDeviceCommandList);
                    secondDeviceCommandQueue->WaitForFenceValue(fenceValueSSR);

                    auto end = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    std::cout << duration << ": DrawSSR" << std::endl;
                }

                {
                    auto start = std::chrono::steady_clock::now();

                    ComPtr<ID3D12GraphicsCommandList2> secondDeviceCommandList = secondDeviceCommandQueue->GetCommandList();

                    CATR.CopySecondDeviceDataToSharedMemory(secondDeviceCommandList);

                    uint64_t fenceValueSSR = secondDeviceCommandQueue->ExecuteCommandList(secondDeviceCommandList);
                    secondDeviceCommandQueue->WaitForFenceValue(fenceValueSSR);
                    
                    auto end = std::chrono::steady_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                    std::cout << duration << ": CopySecondDeviceDataToSharedMemory" << std::endl;
                }
            }
        }
    );

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();

    {
        auto start = std::chrono::steady_clock::now();

        ComPtr<ID3D12GraphicsCommandList2> primaryDeviceCommandList = primaryDeviceCommandQueue->GetCommandList();

        DrawSceneToShadowMaps(primaryDeviceCommandList);
        DrawSceneToGBuffer(primaryDeviceCommandList);
        LightPassRender(primaryDeviceCommandList);

        uint64_t fenceValue = primaryDeviceCommandQueue->ExecuteCommandList(primaryDeviceCommandList);
        primaryDeviceCommandQueue->WaitForFenceValue(fenceValue);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << duration << ": ShadowMap + GBuffer + LightPass" << std::endl;
    }

    {
        auto start = std::chrono::steady_clock::now();

        ComPtr<ID3D12GraphicsCommandList2> primaryDeviceCommandList = primaryDeviceCommandQueue->GetCommandList();

        CATR.CopyPrimaryDeviceDataToSharedMemory(primaryDeviceCommandList);

        uint64_t fenceValue = primaryDeviceCommandQueue->ExecuteCommandList(primaryDeviceCommandList);
        primaryDeviceCommandQueue->WaitForFenceValue(fenceValue);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << duration << ": CopyPrimaryDeviceDataToSharedMemory" << std::endl;
    }

    if (!m_IsFirstFrame)
    {
        auto start = std::chrono::steady_clock::now();

        ComPtr<ID3D12GraphicsCommandList2> primaryDeviceCommandList = primaryDeviceCommandQueue->GetCommandList();

        CATR.CopySharedMemoryDataToPrimaryDevice(primaryDeviceCommandList);

        uint64_t fenceValue = primaryDeviceCommandQueue->ExecuteCommandList(primaryDeviceCommandList);
        primaryDeviceCommandQueue->WaitForFenceValue(fenceValue);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << duration << ": CopySharedMemoryDataToPrimaryDevice" << std::endl;
    }

    ComPtr<ID3D12GraphicsCommandList2> primaryDeviceCommandList = primaryDeviceCommandQueue->GetCommandList();

    MergeResults(primaryDeviceCommandList);

    /*m_FenceValues[currentBackBufferIndex] = primaryDeviceCommandQueue->ExecuteCommandList(primaryDeviceCommandList);
    currentBackBufferIndex = m_pWindow->Present();
    primaryDeviceCommandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);*/

    uint64_t fenceValue = primaryDeviceCommandQueue->ExecuteCommandList(primaryDeviceCommandList);
    primaryDeviceCommandQueue->WaitForFenceValue(fenceValue);

    m_pWindow->Present();

    ssrThread.join();

    m_IsFirstFrame = false;
}

void MultiGpuGame::DrawMultiGpu()
{
    shared_ptr<CommandQueue> primaryDeviceCommandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    shared_ptr<CommandQueue> secondDeviceCommandQueue = Application::Get().GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

    // primary device

    ComPtr<ID3D12GraphicsCommandList2> primaryDeviceCommandList = primaryDeviceCommandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();

    if (!m_IsFirstFrame)
        CATR.CopySharedMemoryDataToPrimaryDevice(primaryDeviceCommandList);

    DrawSceneToShadowMaps(primaryDeviceCommandList);
    DrawSceneToGBuffer(primaryDeviceCommandList);
    LightPassRender(primaryDeviceCommandList);
    CATR.CopyPrimaryDeviceDataToSharedMemory(primaryDeviceCommandList);

    MergeResults(primaryDeviceCommandList);

    uint64_t fenceValuePrimary = primaryDeviceCommandQueue->ExecuteCommandList(primaryDeviceCommandList);
    
    // second device

    ComPtr<ID3D12GraphicsCommandList2> secondDeviceCommandList = secondDeviceCommandQueue->GetCommandList();
    if (!m_IsFirstFrame)
    {
        CATR.CopySharedMemoryDataToSecondDevice(secondDeviceCommandList);
        DrawSSR(
            secondDeviceCommandList,
            GraphicAdapterSecond,
            CATR.SecondDepthBuffer,
            CATR.SecondGBuffer,
            CATR.SecondLightPassResult,
            CATR.SecondSSRResult,
            m_SSRPipelineSecondDevice);
        CATR.CopySecondDeviceDataToSharedMemory(secondDeviceCommandList);
    }
    uint64_t fenceValueSecond = secondDeviceCommandQueue->ExecuteCommandList(secondDeviceCommandList);

    // wait
    
    primaryDeviceCommandQueue->WaitForFenceValue(fenceValuePrimary);
    secondDeviceCommandQueue->WaitForFenceValue(fenceValueSecond);

    

    m_pWindow->Present();

    m_IsFirstFrame = false;
}

void MultiGpuGame::OnRender(RenderEventArgs& e)
{
    if (!m_Initialized) return;
    
    super::OnRender(e);

    if (m_IsMultiGpuRender)
    {
        TestTime("../../DX12GE/Resources/multi_gpu.txt", static_cast<float>(e.ElapsedTime));
        DrawMultiGpu();
    }
    else
    {
        TestTime("../../DX12GE/Resources/single_gpu.txt", static_cast<float>(e.ElapsedTime));
        DrawSingleGpu();
    }       

    CurrentPass::Set(CurrentPass::None);
}

void MultiGpuGame::TestTime(string outputFile, float elapsedTime)
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

void MultiGpuGame::OnKeyPressed(KeyEventArgs& e)
{
    super::OnKeyPressed(e);
    
    m_Camera.OnKeyPressed(e);

    switch (e.Key)
    {
    case KeyCode::Up:
        m_IsMultiGpuRender = true;
        m_IsFirstFrame = true;
        break;
    case KeyCode::Down:
        m_IsMultiGpuRender = false;
        break;
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
    case KeyCode::T:
        m_Camera.StartTest();
        IsTesting = true;
        break;    
    case KeyCode::R:
        auto commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        auto commandList = commandQueue->GetCommandList();
        particles.SpawnParticleGroup(commandList, boxPosition + boxSize * 0.5f, 7.0f, 1000.0f);
        uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
        commandQueue->WaitForFenceValue(fenceValue);
        break;
    
    }    
}

void MultiGpuGame::OnKeyReleased(KeyEventArgs& e)
{
    m_Camera.OnKeyReleased(e);
}

void MultiGpuGame::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_Camera.OnMouseWheel(e);
}

void MultiGpuGame::OnMouseMoved(MouseMotionEventArgs& e)
{
    m_Camera.OnMouseMoved(e);
}

void MultiGpuGame::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    m_Camera.OnMouseButtonPressed(e);
}

void MultiGpuGame::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    m_Camera.OnMouseButtonReleased(e);
}

void MultiGpuGame::OnResize(ResizeEventArgs& e)
{
    if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
    {
        super::OnResize(e);
        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));
        CATR.Resize(e.Width, e.Height);       
    }

    m_Camera.Ratio = static_cast<float>(e.Width) / static_cast<float>(e.Height);
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

        std::wstring mg = m_IsMultiGpuRender ? L" | Multi-GPU" : L" | Single-GPU";
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

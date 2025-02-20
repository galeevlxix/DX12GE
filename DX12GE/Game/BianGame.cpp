#include "BianGame.h"

#include "../Engine/Application.h"
#include "../Engine/CommandQueue.h"

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using namespace DirectX;

// Clamp a value between a min and max range
template<typename T>
constexpr const T& clamp(const T& val, const T& min, const T& max)
{
    return val < min ? min : val > max ? max : val;
}

BianGame::BianGame(const std::wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
    , m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX))
    , m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)))
    , m_FoV(60.0)
    , m_ContentLoaded(false)
{
}

bool BianGame::LoadContent()
{
    auto device = Application::Get().GetDevice();
    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    //x: -18 .. 18
    //y: -10 .. 10
    //z: 10

    lRacket.OnLoad(commandList, true);
    rRacket.OnLoad(commandList, false);

    ball.OnLoad(commandList);

    wall.OnLoad(commandList);

    // Create the descriptor heap for the depth-stencil view
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(
        device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

    // Load the vertex shader
    ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed(
        D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

    // Load the pixel shader
    ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed(
        D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));

    // Create the vertex input layout
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Create a root signature
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    // Serialize the root signature
    ComPtr<ID3DBlob> rootSignatureBlob;
    ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(
        D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

    // Create the root signature
    ThrowIfFailed(
        device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
    } pipelineStateStream;

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    pipelineStateStream.pRootSignature = m_RootSignature.Get();
    pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateStream.RTVFormats = rtvFormats;

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = 
    {
        sizeof(PipelineStateStream), &pipelineStateStream
    };
    ThrowIfFailed(
        device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

    auto fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    m_ContentLoaded = true;

    // Resize/Create the depth buffer
    ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

    return true;
}

void BianGame::ResizeDepthBuffer(int width, int height)
{
    if (m_ContentLoaded)
    {
        // Flush any GPU commands that might be referencing the depth buffer
        Application::Get().Flush();

        width = std::max(1, width);
        height = std::max(1, height);

        auto device = Application::Get().GetDevice();

        // Resize screen dependent resources
        // Create a depth buffer
        D3D12_CLEAR_VALUE optimizedClearValue = {};
        optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        optimizedClearValue.DepthStencil = { 1.0f, 0 };

        auto r5 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto r6 = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        ThrowIfFailed(
            device->CreateCommittedResource(&r5, D3D12_HEAP_FLAG_NONE, &r6, D3D12_RESOURCE_STATE_DEPTH_WRITE, &optimizedClearValue, IID_PPV_ARGS(&m_DepthBuffer)));

        // Update the depth-stencil view
        D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv.Texture2D.MipSlice = 0;
        dsv.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv, m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }
}

void BianGame::OnResize(ResizeEventArgs& e)
{
    if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
    {
        super::OnResize(e);
        m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));
        ResizeDepthBuffer(e.Width, e.Height);
    }
}

void BianGame::UnloadContent()
{
    m_ContentLoaded = false;
}

void BianGame::OnUpdate(UpdateEventArgs& e)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;

    super::OnUpdate(e);

    totalTime += e.ElapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

        //char buffer[512];
        //sprintf_s(buffer, "FPS: %f\n", fps);
        //OutputDebugStringA(buffer);

        frameCount = 0;
        totalTime = 0.0;
    }

    lRacket.OnUpdate(e.ElapsedTime);
    rRacket.OnUpdate(e.ElapsedTime);
    
    ball.OnUpdate(e.ElapsedTime, &lRacket, &rRacket);

    wall.OnUpdate(e.ElapsedTime);

    // Update the view matrix
    const XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
    const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
    const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
    m_ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    // Update the projection matrix
    float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
    m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FoV), aspectRatio, 0.1f, 100.0f);
}

// Transition a resource
void BianGame::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);
    commandList->ResourceBarrier(1, &barrier);
}

// Clear a render target
void BianGame::ClearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
    commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void BianGame::ClearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
    commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void BianGame::OnRender(RenderEventArgs& e)
{
    super::OnRender(e);

    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    auto backBuffer = m_pWindow->GetCurrentBackBuffer();
    auto rtv = m_pWindow->GetCurrentRenderTargetView();
    auto dsv = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

    // Clear the render targets.
    {
        TransitionResource(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        //FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        ClearRTV(commandList, rtv, clearColor);
        ClearDepth(commandList, dsv);
    }

    commandList->SetPipelineState(m_PipelineState.Get());
    commandList->SetGraphicsRootSignature(m_RootSignature.Get());

    commandList->RSSetViewports(1, &m_Viewport);
    commandList->RSSetScissorRects(1, &m_ScissorRect);

    commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    lRacket.OnRender(commandList, m_ViewMatrix, m_ProjectionMatrix);
    rRacket.OnRender(commandList, m_ViewMatrix, m_ProjectionMatrix);

    ball.OnRender(commandList, m_ViewMatrix, m_ProjectionMatrix);

    wall.OnRender(commandList, m_ViewMatrix, m_ProjectionMatrix);

    // Present
    {
        TransitionResource(commandList, backBuffer,D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
        currentBackBufferIndex = m_pWindow->Present();
        commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
    }
}

void BianGame::OnKeyPressed(KeyEventArgs& e)
{
    super::OnKeyPressed(e);

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

    case KeyCode::W:
        lRacket.Move(0, 1);
        break;
    case KeyCode::S:
        lRacket.Move(0, -1);
        break;
    case KeyCode::A:
        lRacket.Move(-1, 0);
        break;
    case KeyCode::D:
        lRacket.Move(1, 0); 
        break;

    case KeyCode::Up:
        rRacket.Move(0, 1);
        break;
    case KeyCode::Down:
        rRacket.Move(0, -1);
        break;
    case KeyCode::Left:
        rRacket.Move(-1, 0);
        break;
    case KeyCode::Right:
        rRacket.Move(1, 0);
        break;
    }
}

void BianGame::OnMouseWheel(MouseWheelEventArgs& e)
{
    m_FoV -= e.WheelDelta;
    m_FoV = clamp(m_FoV, 12.0f, 90.0f);

    char buffer[256];
    sprintf_s(buffer, "FoV: %f\n", m_FoV);
    OutputDebugStringA(buffer);
}


bool Racket::CheckXBorder(float dx, bool isLeft)
{
    if (isLeft)
    {
        return !((dx < 0 && GetPosition().X == -borderX) || (dx > 0 && GetPosition().X == -1));
    }
    return !((dx > 0 && GetPosition().X == borderX) || (dx < 0 && GetPosition().X == 1));
}

bool Racket::CheckYBorder(float dy)
{
    return !((dy > 0 && cubes[length - 1].GetPosition().Y == borderY) || (dy < 0 && cubes[0].GetPosition().Y == -borderY));
}

void Racket::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, bool left)
{
    float xPos = left ? -borderX : borderX;
    float yPos = 0;

    for (int i = 0; i < length; i++)
    {
        cubes[i].OnLoad(commandList, xPos, yPos + i - 2, 10, true);
    }
}

void Racket::OnUpdate(double deltaTime)
{
    for (int i = 0; i < length; i++)
    {
        cubes[i].OnUpdate(deltaTime);
    }
}

void Racket::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
    for (int i = 0; i < length; i++)
    {
        cubes[i].OnRender(commandList, viewMatrix, projectionMatrix);
    }
}

void Racket::Move(float dx, float dy)
{
    if (dy != 0 && CheckYBorder(dy))
    {
        for (int i = 0; i < length; i++)
        {
            cubes[i].Move(0, dy, 0);
        }
    }       

    if (dx != 0 && CheckXBorder(dx, GetPosition().X < 0))
    {
        for (int i = 0; i < length; i++)
        {
            cubes[i].Move(dx, 0, 0);
        }
    }
}

void Racket::SetStartPosition()
{
    for (int i = 0; i < length; i++)
    {
        cubes[i].SetPosition(GetPosition().X < 0 ? -borderX : borderX, i - 2, cubes[i].GetPosition().Z);
    }
}

Vector3 Racket::GetNewDirection(Vector3 ballPos, Vector3 direction)
{
    float coef = ballPos.Y - GetPosition().Y;
    Vector3 newDir;
    newDir.Set(-direction.X, coef / 2.5f, direction.Z);
    return newDir;
}

Vector3 Racket::GetPosition()
{
    return cubes[2].GetPosition();
}

void Ball::CheckYBorder()
{
    if ((direction.Y > 0 && cube.GetPosition().Y >= borderY) || (direction.Y < 0 && cube.GetPosition().Y <= -borderY))
    {
        direction.Y = -direction.Y;
    }       
}

void Ball::CheckRackets(Racket* left, Racket* right)
{
    if (direction.X < 0 && 
        cube.GetPosition().X <= left->GetPosition().X + 1 && 
        cube.GetPosition().X >= left->GetPosition().X - 1 &&
        cube.GetPosition().Y <= left->GetPosition().Y + 2.5f &&
        cube.GetPosition().Y >= left->GetPosition().Y - 2.5f)
    {
        direction = left->GetNewDirection(cube.GetPosition(), direction);
    }
    else if (direction.X > 0 &&
        cube.GetPosition().X >= right->GetPosition().X - 1 &&
        cube.GetPosition().X <= right->GetPosition().X + 1 &&
        cube.GetPosition().Y <= right->GetPosition().Y + 2.5f &&
        cube.GetPosition().Y >= right->GetPosition().Y - 2.5f)
    {
        direction = right->GetNewDirection(cube.GetPosition(), direction);
    }
    else return;

    speed += 1;
}

int GetRandomNumber(int start, int end)
{
    return rand() % (end - start + 1) + start;
}

Vector3 GetRandomDirection()
{
    Vector3 out;
    float randNum = (float)GetRandomNumber(-100, 100);
    randNum /= 100.0;
    bool randNum2 = GetRandomNumber(-100, 100) < 0 ? false : true;
    out.Set(randNum2 ? 1 : -1, randNum, 0);
    return out;
}

void Ball::CheckXBorder(Racket* left, Racket* right)
{
    static int leftScore = 0;
    static int rightScore = 0;

    if (direction.X < 0 && cube.GetPosition().X <= -borderX)
    {
        rightScore++;
    }
    else if (direction.X > 0 && cube.GetPosition().X >= borderX)
    {
        leftScore++;
    }
    else return;

    char buffer[512];
    sprintf_s(buffer, "яв╗р %d:%d\n", leftScore, rightScore);
    OutputDebugStringA(buffer);

    left->SetStartPosition();
    right->SetStartPosition();
    
    cube.SetPosition(0, 0, 10);
    Vector3 newDir = GetRandomDirection();
    direction.Set(newDir.X, newDir.Y, newDir.Z);

    speed = 6;
}

void Ball::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    srand(time(0));
    cube.OnLoad(commandList, 0, 0, 10, false);
    Vector3 newDir = GetRandomDirection();
    direction.Set(newDir.X, newDir.Y, newDir.Z);   
}

void Ball::OnUpdate(double deltaTime, Racket* left, Racket* right)
{
    CheckYBorder();
    CheckRackets(left, right);
    CheckXBorder(left, right);
    Vector3 moveVec = direction * speed * deltaTime;
    cube.Move(moveVec.X, moveVec.Y, moveVec.Z);
    cube.OnUpdate(deltaTime);
}

void Ball::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
    cube.OnRender(commandList, viewMatrix, projectionMatrix);
}

void Wall::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    float centerX = 0;
    float centerY = 0;

    for (int i = 0; i < (borderX + 1) * 2 + 1; i += 2)
    {
        cubes[i].OnLoad(commandList, centerX - i + 19, borderY + 1, 10, true);
        cubes[i].SetScale(0.25, 0.25, 0.25);
        cubes[i + 1].OnLoad(commandList, centerX - i + 19, -borderY - 1, 10, true);
        cubes[i + 1].SetScale(0.25, 0.25, 0.25);

        char buffer[512];
        sprintf_s(buffer, "%d\n", i + 1);
        OutputDebugStringA(buffer);
    }

    
}

void Wall::OnUpdate(double deltaTime)
{
    for (int i = 0; i < 39; i += 2)
    {
        cubes[i].OnUpdate(deltaTime);
        cubes[i + 1].OnUpdate(deltaTime);
    }
}

void Wall::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
    for (int i = 0; i < 39; i += 2)
    {
        cubes[i].OnRender(commandList, viewMatrix, projectionMatrix);
        cubes[i + 1].OnRender(commandList, viewMatrix, projectionMatrix);
    }
}

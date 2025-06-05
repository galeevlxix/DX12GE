#include "../Mesh3D.h"
#include "../../Base/Application.h"
#include "../ShaderResources.h"

Matrix BaseObjectShadowMapView[CASCADES_COUNT];
bool debugMatrix = false;

bool g_IsShadowPass = false;
bool g_IsGeometryPass = false;
bool g_IsLightPass = false;

void Mesh3D::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetDevice();

    size_t bufferSize = numElements * elementSize;

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    // Create a committed resource for the GPU resource in a default heap
    ThrowIfFailed(
        device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(pDestinationResource)));

    // Create an committed resource for the upload
    if (bufferData)
    {
        auto r3 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto r4 = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        ThrowIfFailed(
            device->CreateCommittedResource(&r3, D3D12_HEAP_FLAG_NONE, &r4, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(), *pDestinationResource, *pIntermediateResource, 0, 0, 1, &subresourceData);
    }
}



void Mesh3D::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    if (!IsInitialized()) return;

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    // Update the MVP matrix
    XMMATRIX mvp = XMMatrixMultiply(m_WorldMatrix, viewProjMatrix);

    if (g_IsShadowPass)
    {  
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvp, 0);
    }
    else if (g_IsGeometryPass)
    {
        ShaderResources::GetObjectCB()->WorldViewProjection = m_WorldMatrix;
        ShaderResources::GetObjectCB()->ModelViewProjection = mvp;
        ShaderResources::SetGraphicsObjectCB(commandList, 0);

        for (size_t i = 0; i < CASCADES_COUNT; i++)
        {
            ShaderResources::GetShadowCB()->ShadowTransforms[i] = BaseObjectShadowMapView[i];
        }
    }
    else { }    
    
    commandList->DrawIndexedInstanced(indiciesCount, 1, 0, 0, 0);
}

void Mesh3D::OnRenderLineList(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    if (!IsInitialized()) return;

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    XMMATRIX mvpMatrix = XMMatrixMultiply(m_WorldMatrix, viewProjMatrix);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

    commandList->DrawIndexedInstanced(indiciesCount, 1, 0, 0, 0);
}

void Mesh3D::OnRenderPointList(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (!IsInitialized()) return;

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    commandList->DrawIndexedInstanced(indiciesCount, 1, 0, 0, 0);
}

void Mesh3D::SetSMMatrices(Matrix m[])
{
    for (int i = 0; i < CASCADES_COUNT; i++)
    {
        BaseObjectShadowMapView[i] = m[i];
    }
}

void Mesh3D::DebugMatrices()
{
    debugMatrix = true;
}

void Mesh3D::SetShadowPass(bool isShadowPass)
{
    g_IsShadowPass = isShadowPass;
}

bool Mesh3D::GetShadowPass()
{
    return g_IsShadowPass;
}

void Mesh3D::SetGeometryPass(bool isGeometryPass)
{
    g_IsGeometryPass = isGeometryPass;
}

bool Mesh3D::GetGeometryPass()
{
    return g_IsGeometryPass;
}

void Mesh3D::SetLightPass(bool isLightPass)
{
    g_IsLightPass = isLightPass;
}

bool Mesh3D::GetLightPass()
{
    return g_IsLightPass;
}

void Mesh3D::Release()
{
    if (!Initialized) return;
    Initialized = false;
    m_VertexBuffer->Release();
    m_VertexBuffer = nullptr;
    IntermediateVertexBufferResource->Release();
    IntermediateVertexBufferResource = nullptr;
    m_IndexBuffer->Release();
    m_IndexBuffer = nullptr;
    IntermediateIndexBufferResource->Release();
    IntermediateIndexBufferResource = nullptr;
}

void Mesh3D::OnUpdate()
{
    if (!IsInitialized()) return;

    m_WorldMatrix =
        XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
        XMMatrixRotationX(m_Rotation.x) *
        XMMatrixRotationY(m_Rotation.y) *
        XMMatrixRotationZ(m_Rotation.z) *
        XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
}

void Mesh3D::OnUpdateByRotationMatrix(double deltaTime, XMMATRIX rotMat)
{
    if (!IsInitialized()) return;

    m_WorldMatrix = 
        XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
        rotMat * 
        XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
}

void Mesh3D::SetDefaultState()
{
    SetPosition(Vector3(0, 0, 0));
    SetRotation(Vector3(0, 0, 0));
    SetScale(Vector3(1, 1, 1));
}

void Mesh3D::SetPosition(float x, float y, float z)
{
    m_Position = Vector3(x, y, z);
}

void Mesh3D::SetPosition(Vector3 Position)
{
    m_Position = Position;
}

void Mesh3D::Move(float dx, float dy, float dz)
{
    m_Position += Vector3(dx, dy, dz);
}

void Mesh3D::Move(Vector3 MoveVector)
{
    m_Position = m_Position + MoveVector;
}

void Mesh3D::SetRotation(float angleX, float angleY, float angleZ)
{
    SetRotationX(angleX);
    SetRotationY(angleY);
    SetRotationZ(angleZ);
}

void Mesh3D::SetRotation(Vector3 RotationVector)
{
    m_Rotation = RotationVector;
}

void Mesh3D::SetRotationX(float angleX)
{
    m_Rotation.x = angleX;
}

void Mesh3D::SetRotationY(float angleY)
{
    m_Rotation.y = angleY;
}

void Mesh3D::SetRotationZ(float angleZ)
{
    m_Rotation.z = angleZ;
}

void Mesh3D::Rotate(Vector3 RotateVector)
{
    m_Rotation = m_Rotation + RotateVector;
}

void Mesh3D::SetScale(float x, float y, float z)
{
    m_Scale = Vector3(x, y, z);
}

void Mesh3D::SetScale(Vector3 ScaleVector)
{
    m_Scale = ScaleVector;
}

void Mesh3D::Expand(float value)
{
    m_Scale = m_Scale * value;
}

Vector3 Mesh3D::GetPosition()
{
    return m_Position;
}

Vector3 Mesh3D::GetRotation()
{
    return m_Rotation;
}

Vector3 Mesh3D::GetScale()
{
    return m_Scale;
}


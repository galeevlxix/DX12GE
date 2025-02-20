#include "BaseObject.h"

#include "../Engine/Application.h"

#define PI 3.1415926535f

void BaseObject::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetDevice();

    size_t bufferSize = numElements * elementSize;

    auto r1 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto r2 = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    // Create a committed resource for the GPU resource in a default heap.
    ThrowIfFailed(
        device->CreateCommittedResource(&r1, D3D12_HEAP_FLAG_NONE, &r2, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(pDestinationResource)));

    // Create an committed resource for the upload.
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

void BaseObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, float x, float y, float z, bool isWhite)
{
    if (isWhite)
    {
        for (int i = 0; i < 8; i++)
        {
            m_Vertices[i].Color = XMFLOAT3(1, 1, 1);
        }
    }

    // Загрузить данные вершинного буфера
    UpdateBufferResource(commandList, &m_VertexBuffer, &intermediateVertexBuffer, _countof(m_Vertices), sizeof(VertexPosColor), m_Vertices);

    // Создать представление буфера вершин
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = sizeof(m_Vertices);
    m_VertexBufferView.StrideInBytes = sizeof(VertexPosColor);

    // Загрузить данные индексного буфера
    UpdateBufferResource(commandList, &m_IndexBuffer, &intermediateIndexBuffer, _countof(m_Indicies), sizeof(WORD), m_Indicies);

    // Создать представление индексного буфера
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = sizeof(m_Indicies);

    SetPosition(x, y, z);
    SetRotation(0, 0, 0);
    SetScale(0.5f, 0.5f, 0.5f);
}

void BaseObject::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);
    commandList->ResourceBarrier(1, &barrier);
}

void BaseObject::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    // Update the MVP matrix
    XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, viewMatrix);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, projectionMatrix);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

    commandList->DrawIndexedInstanced(_countof(m_Indicies), 1, 0, 0, 0);
}

void BaseObject::OnUpdate(double deltaTime)
{
    m_ModelMatrix =
        XMMatrixScaling(m_Scale.X, m_Scale.Y, m_Scale.Z) *
        XMMatrixRotationX(m_Rotation.X) *
        XMMatrixRotationY(m_Rotation.Y) *
        XMMatrixRotationZ(m_Rotation.Z) *
        XMMatrixTranslation(m_Position.X, m_Position.Y, m_Position.Z);
}

void BaseObject::SetPosition(float x, float y, float z)
{
    m_Position.Set(x, y, z);
}

void BaseObject::Move(float dx, float dy, float dz)
{
    m_Position.Increase(dx, dy, dz);
}

void BaseObject::SetRotation(float angleX, float angleY, float angleZ)
{
    SetRotationX(angleX);
    SetRotationY(angleY);
    SetRotationZ(angleZ);
}

void BaseObject::SetRotationX(float angleX)
{
    m_Rotation.X = angleX;
}

void BaseObject::SetRotationY(float angleY)
{
    m_Rotation.Y = angleY;
}

void BaseObject::SetRotationZ(float angleZ)
{
    m_Rotation.Z = angleZ;
}

void BaseObject::SetScale(float x, float y, float z)
{
    m_Scale.Set(x, y, z);
}

Vector3 BaseObject::GetPosition()
{
    return m_Position;
}

Vector3 BaseObject::GetRotation()
{
    return m_Rotation;
}

Vector3 BaseObject::GetScale()
{
    return m_Scale;
}

void Vector3::Set(float x, float y, float z)
{
    X = x;
    Y = y;
    Z = z;
}

void Vector3::Increase(float dx, float dy, float dz)
{
    X += dx;
    Y += dy;
    Z += dz;
}

void Vector3::Normalize()
{
    float length = X * X + Y * Y + Z * Z;
    if (length != 0)
    {
        Set(X / length, Y / length, Z / length);
    }
}

Vector3 Vector3::operator*(float value)
{
    Vector3 out;
    out.Set(X * value, Y * value, Z * value);
    return out;
}

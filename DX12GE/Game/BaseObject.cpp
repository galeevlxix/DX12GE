#include "BaseObject.h"

#include "../Engine/Application.h"

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

        UpdateSubresources(commandList.Get(),
            *pDestinationResource, *pIntermediateResource,
            0, 0, 1, &subresourceData);
    }
}

void BaseObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
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

    SetPosition(-2, 0, 0);
    SetRotation(0, 0, 0);
    SetScale(1, 1, 1);
}

void BaseObject::OnUpdate(double totalTime)
{
   // float rotation_speed = 180.0f;
    //float angle = static_cast<float>(totalTime * rotation_speed);
    //const XMVECTOR rotationAxis = XMVectorSet(1, 0, 0, 0);
    //m_ModelMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));

    m_ModelMatrix = m_ScaleMatrix * m_RotationXMatrix * m_RotationYMatrix * m_RotationZMatrix * m_PositionMatrix;
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

void BaseObject::SetPosition(float x, float y, float z)
{
    m_PositionMatrix = XMMatrixTranslation(x, y, z);
}

void BaseObject::SetRotation(float angleX, float angleY, float angleZ)
{
    SetRotationX(angleX);
    SetRotationY(angleY);
    SetRotationZ(angleZ);
}

void BaseObject::SetRotationX(float angleX)
{
    m_RotationXMatrix = XMMatrixRotationX(angleX);
}

void BaseObject::SetRotationY(float angleY)
{
    m_RotationYMatrix = XMMatrixRotationY(angleY);
}

void BaseObject::SetRotationZ(float angleZ)
{
    m_RotationZMatrix = XMMatrixRotationZ(angleZ);
}


void BaseObject::SetScale(float x, float y, float z)
{
    m_ScaleMatrix = XMMatrixScaling(x, y, z);
}


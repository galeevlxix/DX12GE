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

        UpdateSubresources(commandList.Get(), *pDestinationResource, *pIntermediateResource, 0, 0, 1, &subresourceData);
    }
}

void BaseObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 Color)
{
    if (m_Vertices.size() == 0) CreateCubeGeometry();

    if (Color.Length() != 0)
    {
        for (int i = 0; i < m_Vertices.size(); i++)
        {
            m_Vertices[i].Color = Color.ToXM();
        }
    }

    // ��������� ������ ���������� ������
    UpdateBufferResource(commandList, &m_VertexBuffer, &intermediateVertexBuffer, m_Vertices.size(), sizeof(VertexPosColor), m_Vertices.data());

    // ������� ������������� ������ ������
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = m_Vertices.size() * sizeof(VertexPosColor);
    m_VertexBufferView.StrideInBytes = sizeof(VertexPosColor);

    // ��������� ������ ���������� ������
    UpdateBufferResource(commandList, &m_IndexBuffer, &intermediateIndexBuffer, m_Indices.size(), sizeof(WORD), m_Indices.data());

    // ������� ������������� ���������� ������
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = m_Indices.size() * sizeof(WORD);

    SetPosition(position);
    SetRotation(rotation);
    SetScale(scale);
}

void BaseObject::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), beforeState, afterState);
    commandList->ResourceBarrier(1, &barrier);
}

void BaseObject::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    // Update the MVP matrix
    XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, viewProjMatrix);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

    commandList->DrawIndexedInstanced(indiciesCount, 1, 0, 0, 0);
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

void BaseObject::SetPosition(Vector3 Position)
{
    m_Position = Position;
}

void BaseObject::Move(float dx, float dy, float dz)
{
    m_Position.Increase(dx, dy, dz);
}

void BaseObject::Move(Vector3 MoveVector)
{
    m_Position = m_Position + MoveVector;
}

void BaseObject::SetRotation(float angleX, float angleY, float angleZ)
{
    SetRotationX(angleX);
    SetRotationY(angleY);
    SetRotationZ(angleZ);
}

void BaseObject::SetRotation(Vector3 RotationVector)
{
    m_Rotation = RotationVector;
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

void BaseObject::Rotate(Vector3 RotateVector)
{
    m_Rotation = m_Rotation + RotateVector;
}

void BaseObject::SetScale(float x, float y, float z)
{
    m_Scale.Set(x, y, z);
}

void BaseObject::SetScale(Vector3 ScaleVector)
{
    m_Scale = ScaleVector;
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

void BaseObject::CreateSphereGeometry(int gx_segments, int gy_segments)
{
    //int gx_segments = 32;
    //int gy_segments = 32;

    XMFLOAT3 colors[8] =
    {
         XMFLOAT3(0.5f, 0.5f, 0.5f),
         XMFLOAT3(1.0f, 0.0f, 0.0f),
         XMFLOAT3(0.0f, 1.0f, 0.0f),
         XMFLOAT3(0.0f, 0.0f, 1.0f),
         XMFLOAT3(1.0f, 1.0f, 0.0f),
         XMFLOAT3(0.0f, 1.0f, 1.0f),
         XMFLOAT3(1.0f, 0.0f, 1.0f),
         XMFLOAT3(1.0f, 1.0f, 1.0f),
    };

    vector<VertexPosColor> vertices;
    int colorIndex = 0;

    for (int y = 0; y <= gy_segments; y++)
    {
        for (int x = 0; x <= gx_segments; x++)
        {
            float xSegment = (float)x / (float)gx_segments;
            float ySegment = (float)y / (float)gy_segments;
            float xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
            float yPos = cos(ySegment * PI);
            float zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);
            vertices.push_back({ XMFLOAT3(xPos, yPos, zPos), colors[colorIndex]});
            colorIndex++;
            colorIndex = colorIndex % 8;
        }
    }
    m_Vertices = vertices;
    
    vector<WORD> indices;
    for (int i = 0; i < gy_segments; i++)
    {
        for (int j = 0; j < gx_segments; j++)
        {
            indices.push_back((i + 1) * (gx_segments + 1) + j + 1);
            indices.push_back((i + 1) * (gx_segments + 1) + j);
            indices.push_back(i * (gx_segments + 1) + j);

            indices.push_back(i * (gx_segments + 1) + j);
            indices.push_back(i * (gx_segments + 1) + j + 1);
            indices.push_back((i + 1) * (gx_segments + 1) + j + 1);
        }
    }
    m_Indices = indices;
    indiciesCount = indices.size();
}

void BaseObject::CreateCubeGeometry()
{
    vector<VertexPosColor> vertices =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },  // 0
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },  // 1
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },   // 2
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },   // 3
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },  // 4
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },  // 5
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },   // 6
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }    // 7
    };
    m_Vertices = vertices;

    vector<WORD> indices =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };
    m_Indices = indices;
    indiciesCount = indices.size();
}

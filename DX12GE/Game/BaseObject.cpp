#include "BaseObject.h"
#include "../Engine/Application.h"

void BaseObject::UpdateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
    auto device = Application::Get().GetDevice();

    size_t bufferSize = numElements * elementSize;

    auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

    // Create a committed resource for the GPU resource in a default heap.
    ThrowIfFailed(
        device->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(pDestinationResource)));

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


void BaseObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, vector<VertexStruct> vertices, vector<WORD> indices)
{
    indiciesCount = static_cast<UINT>(indices.size());

    // Загрузить данные вершинного буфера
    UpdateBufferResource(commandList, &m_VertexBuffer, &IntermediateVertexBufferResource, vertices.size(), sizeof(VertexStruct), vertices.data());

    // Создать представление буфера вершин
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(VertexStruct));
    m_VertexBufferView.StrideInBytes = sizeof(VertexStruct);

    // Загрузить данные индексного буфера
    UpdateBufferResource(commandList, &m_IndexBuffer, &IntermediateIndexBufferResource, indices.size(), sizeof(WORD), indices.data());

    // Создать представление индексного буфера
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(WORD));

    SetPosition(Vector3(0, 0, 0));
    SetRotation(Vector3(0, 0, 0));
    SetScale(Vector3(1, 1, 1));
}

void BaseObject::OnLoadPositionColor(ComPtr<ID3D12GraphicsCommandList2> commandList, vector<VertexPositionColor> vertices, vector<WORD> indices)
{
    indiciesCount = static_cast<UINT>(indices.size());

    // Загрузить данные вершинного буфера
    UpdateBufferResource(commandList, &m_VertexBuffer, &IntermediateVertexBufferResource, vertices.size(), sizeof(VertexPositionColor), vertices.data());
    m_VertexBuffer->SetName(L"m_VertexBuffer");

    // Создать представление буфера вершин
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(VertexPositionColor));
    m_VertexBufferView.StrideInBytes = sizeof(VertexPositionColor);

    // Загрузить данные индексного буфера
    UpdateBufferResource(commandList, &m_IndexBuffer, &IntermediateIndexBufferResource, indices.size(), sizeof(WORD), indices.data());
    m_IndexBuffer->SetName(L"m_IndexBuffer");

    // Создать представление индексного буфера
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(WORD));


    SetPosition(Vector3(0, 0, 0));
    SetRotation(Vector3(0, 0, 0));
    SetScale(Vector3(1, 1, 1));
}

void BaseObject::OnLoadPositionTextCoord(ComPtr<ID3D12GraphicsCommandList2> commandList, vector<VertexPositionTextCoord> vertices, vector<WORD> indices)
{
    indiciesCount = static_cast<UINT>(indices.size());

    // Загрузить данные вершинного буфера
    UpdateBufferResource(commandList, &m_VertexBuffer, &IntermediateVertexBufferResource, vertices.size(), sizeof(VertexPositionTextCoord), vertices.data());

    // Создать представление буфера вершин
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(VertexPositionTextCoord));
    m_VertexBufferView.StrideInBytes = sizeof(VertexPositionTextCoord);

    // Загрузить данные индексного буфера
    UpdateBufferResource(commandList, &m_IndexBuffer, &IntermediateIndexBufferResource, indices.size(), sizeof(WORD), indices.data());

    // Создать представление индексного буфера
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(WORD));

    SetPosition(Vector3(0, 0, 0));
    SetRotation(Vector3(0, 0, 0));
    SetScale(Vector3(1, 1, 1));
}


void BaseObject::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    // Update the MVP matrix
    XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, viewProjMatrix);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &m_ModelMatrix, 0);  
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, sizeof(XMMATRIX) / 4);
    
    commandList->DrawIndexedInstanced(indiciesCount, 1, 0, 0, 0);
}

void BaseObject::OnRenderLineList(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    commandList->IASetIndexBuffer(&m_IndexBufferView);

    XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, viewProjMatrix);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

    commandList->DrawIndexedInstanced(indiciesCount, 1, 0, 0, 0);
}

void BaseObject::OnUpdate()
{
    m_ModelMatrix =
        XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
        XMMatrixRotationX(m_Rotation.x) *
        XMMatrixRotationY(m_Rotation.y) *
        XMMatrixRotationZ(m_Rotation.z) *
        XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
}

void BaseObject::OnUpdateByRotationMatrix(double deltaTime, XMMATRIX rotMat)
{
    m_ModelMatrix = 
        XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
        rotMat * 
        XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
}

void BaseObject::SetPosition(float x, float y, float z)
{
    m_Position = Vector3(x, y, z);
}

void BaseObject::SetPosition(Vector3 Position)
{
    m_Position = Position;
}

void BaseObject::Move(float dx, float dy, float dz)
{
    m_Position += Vector3(dx, dy, dz);
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
    m_Rotation.x = angleX;
}

void BaseObject::SetRotationY(float angleY)
{
    m_Rotation.y = angleY;
}

void BaseObject::SetRotationZ(float angleZ)
{
    m_Rotation.z = angleZ;
}

void BaseObject::Rotate(Vector3 RotateVector)
{
    m_Rotation = m_Rotation + RotateVector;
}

void BaseObject::SetScale(float x, float y, float z)
{
    m_Scale = Vector3(x, y, z);
}

void BaseObject::SetScale(Vector3 ScaleVector)
{
    m_Scale = ScaleVector;
}

void BaseObject::Expand(float value)
{
    m_Scale = m_Scale * value;
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

//void BaseObject::CreateSphereGeometry(int gx_segments, int gy_segments)
//{
//    //float minY = D3D12_FLOAT32_MAX;
//    //int gx_segments = 32;
//    //int gy_segments = 32;
//
//    XMFLOAT3 colors[8] =
//    {
//         XMFLOAT3(0.5f, 0.5f, 0.5f),
//         XMFLOAT3(1.0f, 0.0f, 0.0f),
//         XMFLOAT3(0.0f, 1.0f, 0.0f),
//         XMFLOAT3(0.0f, 0.0f, 1.0f),
//         XMFLOAT3(1.0f, 1.0f, 0.0f),
//         XMFLOAT3(0.0f, 1.0f, 1.0f),
//         XMFLOAT3(1.0f, 0.0f, 1.0f),
//         XMFLOAT3(1.0f, 1.0f, 1.0f),
//    };
//
//    vector<VertexStruct> vertices;
//    int colorIndex = 0;
//
//    for (int y = 0; y <= gy_segments; y++)
//    {
//        for (int x = 0; x <= gx_segments; x++)
//        {
//            double xSegment = (double)x / (double)gx_segments;
//            double ySegment = (double)y / (double)gy_segments;
//            double xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
//            double yPos = cos(ySegment * PI);
//            double zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);
//            vertices.push_back({ XMFLOAT3(xPos, yPos, zPos), colors[colorIndex]});
//            colorIndex++;
//            colorIndex = colorIndex % 8;
//        }
//    }
//    m_Vertices = vertices;
//
//    radius = Vector3(vertices[0].Position.x, vertices[0].Position.y, vertices[0].Position.z).Length();
//    
//    vector<WORD> indices;
//    for (int i = 0; i < gy_segments; i++)
//    {
//        for (int j = 0; j < gx_segments; j++)
//        {
//            indices.push_back((i + 1) * (gx_segments + 1) + j + 1);
//            indices.push_back((i + 1) * (gx_segments + 1) + j);
//            indices.push_back(i * (gx_segments + 1) + j);
//
//            indices.push_back(i * (gx_segments + 1) + j);
//            indices.push_back(i * (gx_segments + 1) + j + 1);
//            indices.push_back((i + 1) * (gx_segments + 1) + j + 1);
//        }
//    }
//    m_Indices = indices;
//    indiciesCount = indices.size();
//}
//
//void BaseObject::CreateCubeGeometry()
//{
//    float minY = D3D12_FLOAT32_MAX;
//
//    vector<VertexStruct> vertices =
//    {
//        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },  // 0
//        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },  // 1
//        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f) },   // 2
//        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },   // 3
//        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },  // 4
//        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f) },  // 5
//        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },   // 6
//        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f) }    // 7
//    };
//    m_Vertices = vertices;
//    //lowerPoint = XMVectorSet(-1.0f, -1.0f, -1.0f, 1);
//    radius = 1;
//
//    vector<WORD> indices =
//    {
//        0, 1, 2, 0, 2, 3,
//        4, 6, 5, 4, 7, 6,
//        4, 5, 1, 4, 1, 0,
//        3, 2, 6, 3, 6, 7,
//        1, 5, 6, 1, 6, 2,
//        4, 0, 3, 4, 3, 7
//    };
//    m_Indices = indices;
//    indiciesCount = indices.size();
//}



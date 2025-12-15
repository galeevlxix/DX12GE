#include "../Application.h"
#include "../CommandQueue.h"
#include "../Singleton.h"
#include "../../Graphics/ResourceStorage.h"

SelectionSystem::SelectionSystem() : m_IdTextureBuffer (nullptr)
{ }

void SelectionSystem::GetObjectIdAt(UINT x, UINT y, UINT* outObjectID)
{
    if (x >= m_IdTextureBuffer->GetWidth() || y >= m_IdTextureBuffer->GetHeight()) {
        printf("ERROR: Coordinates out of bounds!\n");
        *outObjectID = -1;
        return;
    }

    std::shared_ptr<CommandQueue> commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ComPtr<ID3D12GraphicsCommandList2> commandList = commandQueue->GetCommandList();

    m_IdTextureBuffer->SetToCopySource(commandList);

    // Копирование конкретного пикселя
    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = m_IdTextureBuffer->GetResource().Get();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = m_IdTextureBuffer->GetReadbackBuffer().Get();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    UINT numRows;
    UINT64 rowSize, totalBytes;

    D3D12_RESOURCE_DESC resDesc = m_IdTextureBuffer->GetResource()->GetDesc();
    m_IdTextureBuffer->GetDevice()->GetCopyableFootprints(&resDesc, 0, 1, 0, &footprint, &numRows, &rowSize, &totalBytes);

    dstLocation.PlacedFootprint = footprint;

    // Устанавливаем область копирования (1x1 пиксель)
    D3D12_BOX sourceRegion;
    sourceRegion.left = x;
    sourceRegion.right = x + 1;
    sourceRegion.top = y;
    sourceRegion.bottom = y + 1;
    sourceRegion.front = 0;
    sourceRegion.back = 1;

    commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &sourceRegion);

    uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
    commandQueue->WaitForFenceValue(fenceValue);

    // Чтение данных
    UINT* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, sizeof(UINT) };
    ThrowIfFailed(
        m_IdTextureBuffer->GetReadbackBuffer()->Map(0, &readRange, reinterpret_cast<void**>(&mappedData)));

    *outObjectID = *mappedData;

    D3D12_RANGE writeRange = { 0, 0 };
    m_IdTextureBuffer->GetReadbackBuffer()->Unmap(0, &writeRange);
}

void SelectionSystem::DrawDebug()
{
    for (Node3D* obj : m_Selected)
    {
        if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(obj))
        {
            const CollisionBox& box = obj3D->GetCollisionBox();
            Matrix transform = obj->GetWorldMatrix();
            Singleton::GetDebugRender()->DrawBoundingBox(box, transform);
        }       
    }
}

void SelectionSystem::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
	if (e.Button == 1)
	{
		if (!e.Control)
		{
            DeselectAll();
		}

        UINT id = 0;
        GetObjectIdAt(e.X, e.Y, &id);
        id--;

        auto objects = Singleton::GetNodeGraph()->GetAll3DObjects();

        if (id >= objects.size() || id < 0)
        {
            m_Selected.clear();
            return;
        }

        auto obj = std::next(objects.begin(), id);

        bool hasEntity = false;
        for (auto selected : m_Selected)
        {
            if (obj->second == selected)
            {
                hasEntity = true;
                break;
            }
        }

        if (hasEntity) return;

        m_Selected.push_back(obj->second);
	}
}

void SelectionSystem::DeselectAll()
{
    m_Selected.clear();
}

void SelectionSystem::Destroy()
{
    DeselectAll();
    m_IdTextureBuffer.reset();
    m_IdTextureBuffer = nullptr;
}
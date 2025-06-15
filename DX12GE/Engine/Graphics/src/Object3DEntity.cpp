#include "../Object3DEntity.h"
#include <string>
#include "../../Base/Application.h"
#include "../VertexStructures.h"
#include "../AssimpModelLoader.h"
#include "../ShaderResources.h"
#include "../ResourceStorage.h"

void Object3DEntity::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
    AssimpModelLoader modelLoader;

    float yOffset;
    ObjectId = modelLoader.LoadModelData(commandList, filePath, yOffset);
    Transform.SetDefault(yOffset);
}

void Object3DEntity::OnUpdate(double deltaTime )
{
    if (ObjectId == -1) return;
}

void Object3DEntity::SetConstBuffers(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    XMMATRIX wvp = Transform.GetWorldMatrix();
    XMMATRIX mvp = XMMatrixMultiply(wvp, viewProjMatrix);

    if (CurrentPass::Get() == CurrentPass::Shadow)
    {
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvp, 0);
    }
    else if (CurrentPass::Get() == CurrentPass::Geometry)
    {
        ShaderResources::GetObjectCB()->WorldViewProjection = wvp;
        ShaderResources::GetObjectCB()->ModelViewProjection = mvp;
        ShaderResources::SetGraphicsObjectCB(commandList, 0);
    }
    else {}
}

void Object3DEntity::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    if (ObjectId == -1) return;
    SetConstBuffers(commandList, viewProjMatrix);
    ResourceStorage::GetObject3D(ObjectId)->OnRender(commandList);
}


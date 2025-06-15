#pragma once

#include "Mesh3DComponent.h"
#include "MaterialEntity.h"
#include "TransformComponent.h"
#include "CurrentPass.h"
#include <vector>

class Object3DEntity
{
public:
    void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath);
    void OnUpdate(double deltaTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
    TransformComponent Transform;
private:
    void SetConstBuffers(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
    uint32_t ObjectId;
};
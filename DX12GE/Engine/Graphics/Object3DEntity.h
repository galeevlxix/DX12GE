#pragma once

#include "Mesh3DComponent.h"
#include "MaterialEntity.h"
#include "TransformComponent.h"
#include "../AI/AIComponent.h"  // Include the new header
#include "CurrentPass.h"
#include <vector>

class Object3DEntity
{
public:
    TransformComponent Transform;
    std::vector<std::string> _luaClasses;
    bool _isInited = false;
    
private:
    uint32_t ObjectId;
    std::unique_ptr<AIComponent> m_AIComponent;  // Use unique_ptr for ownership

public:
    void AttachAI(AIComponent* ai);  // New method to attach
    virtual void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath);
    virtual void OnUpdate(const double& deltaTime);
    void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, const XMMATRIX& viewProjMatrix);
    virtual void Destroy();
    uint32_t GetId();
    void AddScriptComponent(std::string className);
    std::vector<std::string>& GetEntityScripts();

protected:
    void SetConstBuffers(ComPtr<ID3D12GraphicsCommandList2> commandList, const XMMATRIX& viewProjMatrix);
    
};
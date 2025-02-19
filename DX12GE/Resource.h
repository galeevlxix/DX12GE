#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <memory>

#include <string>

using namespace std;
using namespace Microsoft::WRL;

class Resource
{
public:
    Resource(const wstring& name = L"");
    Resource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue = nullptr, const wstring& name = L"");
    Resource(ComPtr<ID3D12Resource> resource, const wstring& name = L"");
    Resource(const Resource& copy);
    Resource(Resource&& copy);

    Resource& operator=(const Resource& other);
    Resource& operator=(Resource&& other);

    virtual ~Resource();

    // Check to see if the underlying resource is valid.
    bool IsValid() const { return (m_d3d12Resource != nullptr); }

    // Get access to the underlying D3D12 resource
    ComPtr<ID3D12Resource> GetD3D12Resource() const { return m_d3d12Resource; }

    D3D12_RESOURCE_DESC GetD3D12ResourceDesc() const
    {
        D3D12_RESOURCE_DESC resDesc = {};
        if (m_d3d12Resource)
        {
            resDesc = m_d3d12Resource->GetDesc();
        }
        return resDesc;
    }

    // Replace the D3D12 resource
    // Вызывается только из списка команд.
    virtual void SetD3D12Resource(ComPtr<ID3D12Resource> d3d12Resource, const D3D12_CLEAR_VALUE* clearValue = nullptr);
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const = 0;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const = 0;
    void SetName(const wstring& name);

    /**
     * Освободить базовый ресурс. 
     * Это полезно для изменения размера цепочки обмена.
     */
    virtual void Reset();

protected:
    // The underlying D3D12 resource.
    ComPtr<ID3D12Resource> m_d3d12Resource;
    unique_ptr<D3D12_CLEAR_VALUE> m_d3d12ClearValue;
    wstring m_ResourceName;
};


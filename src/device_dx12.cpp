#include "device_dx12.h"

#include "fsrunityplugin.h"


bool DeviceDX12::InternalInit()
{
    if (m_pUnityInterfaces != nullptr) {
        m_pUnityGraphicsD3D12 = m_pUnityInterfaces->Get<IUnityGraphicsD3D12v7>();
        if (m_pUnityGraphicsD3D12 != nullptr) {
            m_pD3D12Device = m_pUnityGraphicsD3D12->GetDevice();
            m_pD3D12Fence = m_pUnityGraphicsD3D12->GetFrameFence();
        }
    }
    return m_pD3D12Device != nullptr;
}

void DeviceDX12::InternalDestroy()
{
    Wait();
    m_pUnityGraphicsD3D12 = nullptr;
    m_pD3D12Device = nullptr;
    m_pD3D12Fence = nullptr;
    for (auto& commandBuffer : m_CommandBufferList) {
        commandBuffer.d3d12CommandAllocator->Release();
        commandBuffer.d3d12CommandList->Release();
    }
}

void* DeviceDX12::GetNativeResource(void* resource, void* desc, uint32_t state, bool observeOnly)
{
    if (resource && desc) {
        *static_cast<D3D12_RESOURCE_DESC*>(desc) = static_cast<ID3D12Resource*>(resource)->GetDesc();
    }
    if (resource) {
        m_ResourceState.push_back(UnityGraphicsD3D12ResourceState{static_cast<ID3D12Resource*>(resource),
            static_cast<D3D12_RESOURCE_STATES>(state),
            static_cast<D3D12_RESOURCE_STATES>(state)});
    }
    return resource;
}

void* DeviceDX12::GetNativeResourceByID(UnityTextureID textureID, void* desc, uint32_t state, bool observeOnly)
{
    ID3D12Resource* resource = nullptr;
    if (m_pUnityGraphicsD3D12 != nullptr) {
        resource = m_pUnityGraphicsD3D12->TextureFromNativeTexture(textureID);
    }
    if (resource && desc) {
        *static_cast<D3D12_RESOURCE_DESC*>(desc) = static_cast<ID3D12Resource*>(resource)->GetDesc();
    }
    if (resource) {
        m_ResourceState.push_back(UnityGraphicsD3D12ResourceState{static_cast<ID3D12Resource*>(resource),
            static_cast<D3D12_RESOURCE_STATES>(state),
            static_cast<D3D12_RESOURCE_STATES>(state)});
    }
    return resource;
}

void* DeviceDX12::GetNativeDevice()
{
    return m_pD3D12Device;
}

void* DeviceDX12::GetNativeCommandList()
{
    ID3D12CommandAllocator* d3d12CommandAllocator = nullptr;
    ID3D12GraphicsCommandList2* d3d12CommandList = nullptr;
    if (m_pD3D12Fence != nullptr) {
        for (auto& commandBuffer : m_CommandBufferList) {
            if (m_pD3D12Fence->GetCompletedValue() >= commandBuffer.fenceValue) {
                d3d12CommandAllocator = commandBuffer.d3d12CommandAllocator;
                d3d12CommandList = commandBuffer.d3d12CommandList;
                commandBuffer.fenceValue = (std::numeric_limits<uint64_t>::max)();
                break;
            }
        }
        if (d3d12CommandAllocator != nullptr && d3d12CommandList != nullptr) {
            HRESULT hr = d3d12CommandAllocator->Reset();
            if (FAILED(hr)) {
                FSR_ERROR("Failed to reset command allocator!");
            }
            hr = d3d12CommandList->Reset(d3d12CommandAllocator, nullptr);
            if (FAILED(hr)) {
                FSR_ERROR("Failed to reset command list!");
            }
        }
    }
    if (!d3d12CommandList) {
        if (m_pD3D12Device != nullptr) {
            HRESULT hr = m_pD3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&d3d12CommandAllocator));
            if (FAILED(hr)) {
                FSR_ERROR("Failed to create command allocator!");
            }
            hr = m_pD3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, d3d12CommandAllocator, nullptr, IID_PPV_ARGS(&d3d12CommandList));
            if (FAILED(hr)) {
                FSR_ERROR("Failed to create command list!");
            }
            m_CommandBufferList.push_back(CommandBuffer{d3d12CommandAllocator, d3d12CommandList, (std::numeric_limits<uint64_t>::max)()});
        }
    }
    return d3d12CommandList;
}

void DeviceDX12::ExecuteCommandList(void* commandList)
{
    static_cast<ID3D12GraphicsCommandList2*>(commandList)->Close();
    if (m_pUnityGraphicsD3D12 != nullptr) {
        //m_pUnityGraphicsD3D12->GetCommandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&commandList));
        uint64_t fenceValue = m_pUnityGraphicsD3D12->ExecuteCommandList(static_cast<ID3D12GraphicsCommandList*>(commandList), static_cast<int>(m_ResourceState.size()), m_ResourceState.data());
        for (auto& commandBuffer : m_CommandBufferList) {
            if (commandList == commandBuffer.d3d12CommandList) {
                commandBuffer.fenceValue = fenceValue;
                break;
            }
        }
    }
}

void DeviceDX12::Wait()
{
    if (m_pD3D12Fence != nullptr) {
        for (auto& commandBuffer : m_CommandBufferList) {
            if (m_pD3D12Fence->GetCompletedValue() < commandBuffer.fenceValue) {
                HANDLE hHandleFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                HRESULT hr = m_pD3D12Fence->SetEventOnCompletion(commandBuffer.fenceValue, hHandleFenceEvent);
                if (FAILED(hr)) {
                    FSR_ERROR("Failed to set event on completion!");
                    break;
                }
                WaitForSingleObject(hHandleFenceEvent, INFINITE);
                CloseHandle(hHandleFenceEvent);
            }
        }
    }
}
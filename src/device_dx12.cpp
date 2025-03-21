#include "device_dx12.h"


bool DeviceDX12::InternalInit()
{
    if (m_pUnityInterfaces != nullptr) {
        const auto unityGraphicsD3D12 = m_pUnityInterfaces->Get<IUnityGraphicsD3D12v7>();
        if (unityGraphicsD3D12 != nullptr) {
            m_pD3D12Device = unityGraphicsD3D12->GetDevice();
            m_pFence = unityGraphicsD3D12->GetFrameFence();
        }
    }
    return m_pD3D12Device != nullptr;
}

void DeviceDX12::InternalDestroy()
{
    Wait();
    m_pD3D12Device = nullptr;
    m_pFence = nullptr;
    for (auto& commandBuffer : m_CommandBufferList) {
        commandBuffer.commandAllocator->Release();
        commandBuffer.commandList->Release();
    }
}

void* DeviceDX12::GetNativeResource(UnityTextureID textureID)
{
    ID3D12Resource* resource = nullptr;
    if (m_pUnityInterfaces != nullptr) {
        const auto unityGraphicsD3D12 = m_pUnityInterfaces->Get<IUnityGraphicsD3D12v7>();
        if (unityGraphicsD3D12 != nullptr) {
            resource = unityGraphicsD3D12->TextureFromNativeTexture(textureID);
        }
    }
    return resource;
}

void DeviceDX12::SetResourceState(void* res, uint32_t state)
{
    m_ResourceState.push_back(UnityGraphicsD3D12ResourceState{static_cast<ID3D12Resource*>(res),
        static_cast<D3D12_RESOURCE_STATES>(state),
        static_cast<D3D12_RESOURCE_STATES>(state)});
}

void* DeviceDX12::GetNativeDevice()
{
    return m_pD3D12Device;
}

void* DeviceDX12::GetNativeCommandList()
{
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12GraphicsCommandList2* commandList = nullptr;
    if (m_pFence != nullptr) {
        for (auto& commandBuffer : m_CommandBufferList) {
            if (m_pFence->GetCompletedValue() >= commandBuffer.fenceValue) {
                commandAllocator = commandBuffer.commandAllocator;
                commandList = commandBuffer.commandList;
            }
        }
    }
    if (!commandList) {
        if (m_pD3D12Device != nullptr) {
            m_pD3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
            m_pD3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
            m_CommandBufferList.push_back(CommandBuffer{commandAllocator, commandList, 0});
        }
    }
    if (commandAllocator != nullptr && commandList != nullptr) {
        commandAllocator->Reset();
        commandList->Reset(commandAllocator, nullptr);
    }
    return commandList;
}

void DeviceDX12::ExecuteCommandList(void* commandList)
{
    static_cast<ID3D12GraphicsCommandList2*>(commandList)->Close();
    if (m_pUnityInterfaces != nullptr) {
        const auto unityGraphicsD3D12 = m_pUnityInterfaces->Get<IUnityGraphicsD3D12v7>();
        if (unityGraphicsD3D12 != nullptr) {
            //unityGraphicsD3D12->GetCommandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&commandList));
            uint64_t fenceValue = unityGraphicsD3D12->ExecuteCommandList(static_cast<ID3D12GraphicsCommandList*>(commandList), static_cast<int>(m_ResourceState.size()), m_ResourceState.data());
            for (auto& commandBuffer : m_CommandBufferList) {
                if (commandList == commandBuffer.commandList) {
                    commandBuffer.fenceValue = fenceValue;
                    break;
                }
            }
        }
    }
}

void DeviceDX12::Wait()
{
    if (m_pFence != nullptr) {
        for (auto& commandBuffer : m_CommandBufferList) {
            if (m_pFence->GetCompletedValue() < commandBuffer.fenceValue) {
                HANDLE hHandleFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
                m_pFence->SetEventOnCompletion(commandBuffer.fenceValue, hHandleFenceEvent);
                WaitForSingleObject(hHandleFenceEvent, INFINITE);
                CloseHandle(hHandleFenceEvent);
            }
        }
    }
}
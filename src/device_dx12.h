#pragma once

#include <vector>

#include <dxgi.h>
#include <d3d12.h>

#include "IUnityGraphicsD3D12.h"
#include "device.h"


class DeviceDX12 : public Device
{
private:
    DeviceDX12() : Device() {}
    friend class Device;

public:
    virtual UnityGfxRenderer GetDeviceType() override { return kUnityGfxRendererD3D12; }
    virtual void* GetGraphicsInterfaces() { return m_pUnityGraphicsD3D12; }
    virtual void* GetNativeResource(void* resource, void* desc = nullptr, uint32_t state = 0, bool observeOnly = true) override;
    virtual void* GetNativeResourceByID(UnityTextureID textureID, void* desc = nullptr, uint32_t state = 0, bool observeOnly = true) override;
    virtual void* GetNativeDevice() override;
    virtual void* GetNativeCommandList() override;
    virtual uint64_t ExecuteCommandList(void* commandList) override;
    virtual void Wait() override;
    virtual void Wait(uint64_t fenceValue) override;

private:
    virtual bool InternalInit() override;
    virtual void InternalDestroy() override;

private:
    IUnityGraphicsD3D12v7* m_pUnityGraphicsD3D12 = nullptr;

    ID3D12Device* m_pD3D12Device = nullptr;
    ID3D12Fence* m_pD3D12Fence = nullptr;

    struct CommandBuffer
    {
        ID3D12CommandAllocator* d3d12CommandAllocator;
        ID3D12GraphicsCommandList2* d3d12CommandList;
        uint64_t fenceValue;
    };
    std::vector<CommandBuffer> m_CommandBufferList = {};

    std::vector<UnityGraphicsD3D12ResourceState> m_ResourceState;
};
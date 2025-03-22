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
    virtual void* GetNativeResource(UnityTextureID textureID) override;
    virtual void SetResourceState(void* res, uint32_t state) override;
    virtual void* GetNativeDevice() override;
    virtual void* GetNativeCommandList() override;
    virtual void ExecuteCommandList(void* commandList) override;
    virtual void Wait() override;

private:
    virtual bool InternalInit() override;
    virtual void InternalDestroy() override;

private:
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
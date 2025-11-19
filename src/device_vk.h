#pragma once

#include <vector>

#include "IUnityGraphics.h"
#include "IUnityGraphicsVulkan.h"
#include "device.h"


class DeviceVK : public Device
{
private:
    DeviceVK() : Device() {}
    friend class Device;

public:
    virtual UnityGfxRenderer GetDeviceType() override { return kUnityGfxRendererVulkan; }
    virtual void* GetGraphicsInterfaces() { return m_pUnityGraphicsVulkan; }
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
    IUnityGraphicsVulkanV2* m_pUnityGraphicsVulkan = nullptr;

    VkDevice m_VkDevice = VK_NULL_HANDLE;
    VkQueue m_VkQueue = VK_NULL_HANDLE;
    //VkSemaphore m_VkSemaphore = VK_NULL_HANDLE;
    uint64_t m_SemaphoreValue = 0;

    struct CommandBuffer
    {
        VkCommandPool vkCommandPool;
        VkCommandBuffer vkCommandBuffer;
        uint64_t semaphoreValue;
        VkFence vkFence;
    };
    std::vector<CommandBuffer> m_CommandBufferList = {};
};
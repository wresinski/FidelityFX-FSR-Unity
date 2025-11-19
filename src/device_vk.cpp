#include "device_vk.h"

#include "fsrunityplugin.h"


bool DeviceVK::InternalInit()
{
    if (m_pUnityInterfaces != nullptr) {
        m_pUnityGraphicsVulkan = m_pUnityInterfaces->Get<IUnityGraphicsVulkanV2>();
        if (m_pUnityGraphicsVulkan != nullptr) {
            m_VkDevice = m_pUnityGraphicsVulkan->Instance().device;
            m_VkQueue = m_pUnityGraphicsVulkan->Instance().graphicsQueue;

            //VkSemaphoreTypeCreateInfo typeCreateInfo = {};
            //typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            //typeCreateInfo.pNext = nullptr;
            //typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            //typeCreateInfo.initialValue = m_SemaphoreValue;

            //VkSemaphoreCreateInfo createInfo = {};
            //createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            //createInfo.pNext = &typeCreateInfo;
            //createInfo.flags = 0;
            //VkResult res = vkCreateSemaphore(m_VkDevice, &createInfo, nullptr, &m_VkSemaphore);
            //if (res != VK_SUCCESS) {
            //    FSR_ERROR("Failed to create queue semaphore!");
            //}
        }
    }
    return m_VkDevice != VK_NULL_HANDLE;
}

void DeviceVK::InternalDestroy()
{
    Wait();
    for (auto& commandBuffer : m_CommandBufferList) {
        vkFreeCommandBuffers(m_VkDevice, commandBuffer.vkCommandPool, 1, &commandBuffer.vkCommandBuffer);
        vkDestroyCommandPool(m_VkDevice, commandBuffer.vkCommandPool, nullptr);
        vkDestroyFence(m_VkDevice, commandBuffer.vkFence, nullptr);
    }
    //if (m_VkSemaphore != VK_NULL_HANDLE) {
    //    vkDestroySemaphore(m_VkDevice, m_VkSemaphore, nullptr);
    //}
    m_VkDevice = VK_NULL_HANDLE;
    m_VkQueue = VK_NULL_HANDLE;
    m_pUnityGraphicsVulkan = nullptr;
}

void* DeviceVK::GetNativeResource(void* resource, void* desc, uint32_t state, bool observeOnly)
{
    UnityVulkanImage vulkanImage = {};
    if (resource) {
        if (m_pUnityGraphicsVulkan != nullptr) {
            VkImageSubresource subResource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};
            bool success = m_pUnityGraphicsVulkan->AccessTexture(
                resource,
                &subResource,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                observeOnly ? kUnityVulkanResourceAccess_ObserveOnly : kUnityVulkanResourceAccess_PipelineBarrier,
                &vulkanImage
            );
            //if (!success) {
            //    FSR_ERROR("AccessTexture failed");
            //}
        }
    }
    if (desc) {
        *static_cast<UnityVulkanImage*>(desc) = vulkanImage;
    }
    return vulkanImage.image;
}

void* DeviceVK::GetNativeResourceByID(UnityTextureID textureID, void* desc, uint32_t state, bool observeOnly)
{
    UnityVulkanImage vulkanImage = {};
    if (m_pUnityGraphicsVulkan != nullptr) {
        VkImageSubresource subResource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};
        bool success = m_pUnityGraphicsVulkan->AccessTextureByID(
            textureID,
            &subResource,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            observeOnly ? kUnityVulkanResourceAccess_ObserveOnly : kUnityVulkanResourceAccess_PipelineBarrier,
            &vulkanImage
        );
        //if (!success) {
        //    FSR_ERROR("AccessTextureByID failed");
        //}
    }
    if (desc) {
        *static_cast<UnityVulkanImage*>(desc) = vulkanImage;
    }
    return vulkanImage.image;
}

void* DeviceVK::GetNativeDevice()
{
    return m_VkDevice;
}

void* DeviceVK::GetNativeCommandList()
{
    VkCommandPool vkCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;
    if (m_VkDevice != VK_NULL_HANDLE) {
        //if (m_VkSemaphore != VK_NULL_HANDLE) {
        //    uint64_t semaphoreValue = 0;
        //    vkGetSemaphoreCounterValue(m_VkDevice, m_VkSemaphore, &semaphoreValue);
        //    for (auto& commandBuffer : m_CommandBufferList) {
        //        if (semaphoreValue >= commandBuffer.semaphoreValue) {
        //            vkCommandPool = commandBuffer.vkCommandPool;
        //            vkCommandBuffer = commandBuffer.vkCommandBuffer;
        //            commandBuffer.semaphoreValue = (std::numeric_limits<uint64_t>::max)();
        //            break;
        //        }
        //    }
        //}

        for (auto& commandBuffer : m_CommandBufferList) {
            VkResult res = vkGetFenceStatus(m_VkDevice, commandBuffer.vkFence);
            if (res == VK_SUCCESS) {
                vkCommandPool = commandBuffer.vkCommandPool;
                vkCommandBuffer = commandBuffer.vkCommandBuffer;
                commandBuffer.semaphoreValue = (std::numeric_limits<uint64_t>::max)();
                break;
            }
        }
    }
    if (vkCommandBuffer == VK_NULL_HANDLE) {
        if (m_VkDevice != VK_NULL_HANDLE) {
            if (m_pUnityGraphicsVulkan != nullptr) {
                VkCommandPoolCreateInfo poolInfo = {};
                poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolInfo.pNext = nullptr;
                poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                poolInfo.queueFamilyIndex = m_pUnityGraphicsVulkan->Instance().queueFamilyIndex;

                VkResult res = vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &vkCommandPool);
                if (res != VK_SUCCESS) {
                    FSR_ERROR("Failed to create queue command pool!");
                }

                VkCommandBufferAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.pNext = nullptr;
                allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandPool = vkCommandPool;
                allocInfo.commandBufferCount = 1;

                res = vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &vkCommandBuffer);
                if (res != VK_SUCCESS) {
                    FSR_ERROR("Failed to allocate a command buffer");
                }

                VkFenceCreateInfo fenceInfo = {};
                fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                VkFence fence = VK_NULL_HANDLE;
                res = vkCreateFence(m_VkDevice, &fenceInfo, nullptr, &fence);
                if (res != VK_SUCCESS) {
                    FSR_ERROR("Failed to create a fence");
                }

                m_CommandBufferList.push_back(CommandBuffer{ vkCommandPool, vkCommandBuffer, (std::numeric_limits<uint64_t>::max)(), fence });
            }
        }
    } else {
        vkResetCommandPool(m_VkDevice, vkCommandPool, 0);
        //vkResetCommandBuffer(vkCommandBuffer, 0);
    }

    if (vkCommandBuffer != VK_NULL_HANDLE) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        VkResult res = vkBeginCommandBuffer(vkCommandBuffer, &beginInfo);
        if (res != VK_SUCCESS) {
            FSR_ERROR("Failed to begin a command buffer");
        }
    }

    return vkCommandBuffer;
}

uint64_t DeviceVK::ExecuteCommandList(void* commandList)
{
    vkEndCommandBuffer(static_cast<VkCommandBuffer>(commandList));

    ++m_SemaphoreValue;

    VkFence fence = VK_NULL_HANDLE;
    for (auto& commandBuffer : m_CommandBufferList) {
        if (commandList == commandBuffer.vkCommandBuffer) {
            commandBuffer.semaphoreValue = m_SemaphoreValue;

            VkResult res = vkResetFences(m_VkDevice, 1, &commandBuffer.vkFence);
            if (res != VK_SUCCESS) {
                FSR_ERROR("Failed to reset fence");
            }

            fence = commandBuffer.vkFence;
            break;
        }
    }

    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext = nullptr;
    info.waitSemaphoreCount = 0;
    info.pWaitSemaphores = nullptr;
    info.pWaitDstStageMask = nullptr;
    info.pCommandBuffers = reinterpret_cast<VkCommandBuffer*>(&commandList);
    info.commandBufferCount = 1;
    //info.signalSemaphoreCount = 1;
    //info.pSignalSemaphores = &m_VkSemaphore;

    //VkTimelineSemaphoreSubmitInfo semaphoreSubmitInfo = {};
    //semaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    //semaphoreSubmitInfo.pNext = nullptr;
    //semaphoreSubmitInfo.waitSemaphoreValueCount = 0;
    //semaphoreSubmitInfo.pWaitSemaphoreValues = nullptr;
    //semaphoreSubmitInfo.signalSemaphoreValueCount = 1;
    //semaphoreSubmitInfo.pSignalSemaphoreValues = &m_SemaphoreValue;

    //info.pNext = &semaphoreSubmitInfo;

    //VkResult res = vkQueueSubmit(m_VkQueue, 1, &info, VK_NULL_HANDLE);
    VkResult res = vkQueueSubmit(m_VkQueue, 1, &info, fence);
    if (res != VK_SUCCESS) {
        FSR_ERROR("Failed to submit queue");
    }

    return m_SemaphoreValue;
}

void DeviceVK::Wait()
{
    if (m_VkDevice != VK_NULL_HANDLE) {
        //if (m_VkSemaphore != VK_NULL_HANDLE) {
        //    for (auto& commandBuffer : m_CommandBufferList) {
        //        VkSemaphoreWaitInfo waitInfo = {};
        //        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        //        waitInfo.pNext = nullptr;
        //        waitInfo.flags = 0;
        //        waitInfo.semaphoreCount = 1;
        //        waitInfo.pSemaphores = &m_VkSemaphore;
        //        waitInfo.pValues = &commandBuffer.semaphoreValue;
        //        VkResult res = vkWaitSemaphores(m_VkDevice, &waitInfo, UINT64_MAX);
        //        if (res != VK_SUCCESS) {
        //            FSR_ERROR("Failed to wait on the queue semaphore.");
        //        }
        //    }
        //}

        std::vector<VkFence> fences = {};
        for (auto& commandBuffer : m_CommandBufferList) {
            fences.push_back(commandBuffer.vkFence);
        }
        VkResult res = vkWaitForFences(m_VkDevice, fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
        if (res != VK_SUCCESS) {
            FSR_ERROR("Failed to wait for fences.");
        }
    }
}

void DeviceVK::Wait(uint64_t fenceValue)
{
    if (m_VkDevice != VK_NULL_HANDLE) {
        //if (m_VkSemaphore != VK_NULL_HANDLE) {
        //    VkSemaphoreWaitInfo waitInfo = {};
        //    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        //    waitInfo.pNext = nullptr;
        //    waitInfo.flags = 0;
        //    waitInfo.semaphoreCount = 1;
        //    waitInfo.pSemaphores = &m_VkSemaphore;
        //    waitInfo.pValues = &fenceValue;
        //    VkResult res = vkWaitSemaphores(m_VkDevice, &waitInfo, UINT64_MAX);
        //    if (res != VK_SUCCESS) {
        //        FSR_ERROR("Failed to wait on the queue semaphore.");
        //    }
        //}

        std::vector<VkFence> fences = {};
        for (auto& commandBuffer : m_CommandBufferList) {
            if (commandBuffer.semaphoreValue <= fenceValue) {
                fences.push_back(commandBuffer.vkFence);
            }
        }
        VkResult res = vkWaitForFences(m_VkDevice, fences.size(), fences.data(), VK_TRUE, UINT64_MAX);
        if (res != VK_SUCCESS) {
            FSR_ERROR("Failed to wait for fences.");
        }
    }
}
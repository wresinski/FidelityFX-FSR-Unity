#pragma once

#include <cstdint>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"


class Device
{
public:
    static Device& Instance(UnityGfxRenderer deviceType = kUnityGfxRendererNull);

protected:
    Device() {}

private:
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(const Device&&) = delete;
    Device& operator=(const Device&&) = delete;

public:
    virtual ~Device() { Destroy(); }
    bool Init(IUnityInterfaces* unityInterfaces);
    void Destroy();
    virtual UnityGfxRenderer GetDeviceType() = 0;
    virtual void* GetGraphicsInterfaces() = 0;
    virtual void* GetNativeResource(void* resource, void* desc = nullptr, uint32_t state = 0, bool observeOnly = true) = 0;
    virtual void* GetNativeResourceByID(UnityTextureID textureID, void* desc = nullptr, uint32_t state = 0, bool observeOnly = true) = 0;
    virtual void* GetNativeDevice() = 0;
    virtual void* GetNativeCommandList() = 0;
    virtual void ExecuteCommandList(void* commandList) {}
    virtual void Wait() {}

private:
    virtual bool InternalInit() = 0;
    virtual void InternalDestroy() = 0;

protected:
    bool m_Initialized = false;
    IUnityInterfaces* m_pUnityInterfaces = nullptr;
};
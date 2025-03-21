#pragma once

#include <cstdint>

#include "IUnityInterface.h"


class Device
{
public:
    static Device& Instance();

protected:
    Device() {}

private:
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(const Device&&) = delete;
    Device& operator=(const Device&&) = delete;

public:
    bool Init(IUnityInterfaces* unityInterfaces);
    void Destroy();
    virtual void* GetNativeResource(UnityTextureID textureID) = 0;
    virtual void SetResourceState(void* res, uint32_t state) {}
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
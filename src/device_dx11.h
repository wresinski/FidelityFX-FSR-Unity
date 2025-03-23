#pragma once

#include <d3d11.h>

#include "IUnityGraphicsD3D11.h"
#include "device.h"


class DeviceDX11 : public Device
{
private:
    DeviceDX11() : Device() {}
    friend class Device;

public:
    virtual UnityGfxRenderer GetDeviceType() override { return kUnityGfxRendererD3D11; }
    virtual void* GetGraphicsInterfaces() { return m_pUnityGraphicsD3D11; }
    virtual void* GetNativeResource(void* resource, void* desc = nullptr, uint32_t state = 0, bool observeOnly = true) override;
    virtual void* GetNativeResourceByID(UnityTextureID textureID, void* desc = nullptr, uint32_t state = 0, bool observeOnly = true) override;
    virtual void* GetNativeDevice() override;
    virtual void* GetNativeCommandList() override;

private:
    virtual bool InternalInit() override;
    virtual void InternalDestroy() override;

private:
    IUnityGraphicsD3D11* m_pUnityGraphicsD3D11 = nullptr;

    ID3D11Device* m_pD3D11Device = nullptr;
    ID3D11DeviceContext* m_pD3D11DeviceContext = nullptr;
};
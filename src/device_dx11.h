#pragma once

#include <d3d11.h>

#include "device.h"


class DeviceDX11 : public Device
{
private:
    DeviceDX11() : Device() {}
    friend class Device;

public:
    virtual UnityGfxRenderer GetDeviceType() override { return kUnityGfxRendererD3D11; }
    virtual void* GetNativeResource(UnityTextureID textureID) override;
    virtual void* GetNativeDevice() override;
    virtual void* GetNativeCommandList() override;

private:
    virtual bool InternalInit() override;
    virtual void InternalDestroy() override;

private:
    ID3D11Device* m_pD3D11Device = nullptr;
    ID3D11DeviceContext* m_pD3D11DeviceContext = nullptr;
};
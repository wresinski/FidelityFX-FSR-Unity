#include "device_dx11.h"

#include "IUnityGraphicsD3D11.h"


bool DeviceDX11::InternalInit()
{
    if (m_pUnityInterfaces != nullptr) {
        const auto unityGraphicsD3D11 = m_pUnityInterfaces->Get<IUnityGraphicsD3D11>();
        if (unityGraphicsD3D11 != nullptr) {
            m_pDevice = unityGraphicsD3D11->GetDevice();
        }
        if (m_pDevice != nullptr) {
            m_pDevice->GetImmediateContext(&m_pDeviceContext);
        }
    }

    return m_pDevice != nullptr;
}

void DeviceDX11::InternalDestroy()
{
    m_pDeviceContext->Release();
    m_pDevice = nullptr;
}

void* DeviceDX11::GetNativeResource(UnityTextureID textureID)
{
    ID3D11Resource* resource = nullptr;
    if (m_pUnityInterfaces != nullptr) {
        const auto unityGraphicsD3D11 = m_pUnityInterfaces->Get<IUnityGraphicsD3D11>();
        if (unityGraphicsD3D11 != nullptr) {
            resource = unityGraphicsD3D11->TextureFromNativeTexture(textureID);
        }
    }
    return resource;
}

void* DeviceDX11::GetNativeDevice()
{
    return m_pDevice;
}

void* DeviceDX11::GetNativeCommandList()
{
    return m_pDeviceContext;
}
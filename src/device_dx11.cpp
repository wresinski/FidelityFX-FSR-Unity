#include "device_dx11.h"

#include "IUnityGraphicsD3D11.h"


bool DeviceDX11::InternalInit()
{
    if (m_pUnityInterfaces != nullptr) {
        const auto unityGraphicsD3D11 = m_pUnityInterfaces->Get<IUnityGraphicsD3D11>();
        if (unityGraphicsD3D11 != nullptr) {
            m_pD3D11Device = unityGraphicsD3D11->GetDevice();
        }
        if (m_pD3D11Device != nullptr) {
            m_pD3D11Device->GetImmediateContext(&m_pD3D11DeviceContext);
        }
    }

    return m_pD3D11Device != nullptr;
}

void DeviceDX11::InternalDestroy()
{
    m_pD3D11DeviceContext->Release();
    m_pD3D11Device = nullptr;
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
    return m_pD3D11Device;
}

void* DeviceDX11::GetNativeCommandList()
{
    return m_pD3D11DeviceContext;
}
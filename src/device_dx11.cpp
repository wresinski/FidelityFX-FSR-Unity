#include "device_dx11.h"


bool DeviceDX11::InternalInit()
{
    if (m_pUnityInterfaces != nullptr) {
        m_pUnityGraphicsD3D11 = m_pUnityInterfaces->Get<IUnityGraphicsD3D11>();
        if (m_pUnityGraphicsD3D11 != nullptr) {
            m_pD3D11Device = m_pUnityGraphicsD3D11->GetDevice();
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

void* DeviceDX11::GetNativeResource(void* resource, void* desc, uint32_t state, bool observeOnly)
{
    if (resource && desc) {
        ID3D11Texture2D* pTexture2D = nullptr;
        static_cast<ID3D11Resource*>(resource)->QueryInterface(IID_ID3D11Texture2D, (void**)&pTexture2D);
        if (pTexture2D) {
            pTexture2D->GetDesc(static_cast<D3D11_TEXTURE2D_DESC*>(desc));
        }
    }
    return resource;
}

void* DeviceDX11::GetNativeResourceByID(UnityTextureID textureID, void* desc, uint32_t state, bool observeOnly)
{
    ID3D11Resource* resource = nullptr;
    if (m_pUnityGraphicsD3D11 != nullptr) {
        resource = m_pUnityGraphicsD3D11->TextureFromNativeTexture(textureID);
    }
    if (resource && desc) {
        ID3D11Texture2D* pTexture2D = nullptr;
        static_cast<ID3D11Resource*>(resource)->QueryInterface(IID_ID3D11Texture2D, (void**)&pTexture2D);
        if (pTexture2D) {
            pTexture2D->GetDesc(static_cast<D3D11_TEXTURE2D_DESC*>(desc));
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
#include "device.h"

#include <memory>

#if defined(FSR_BACKEND_DX11)
#include "device_dx11.h"
#elif defined(FSR_BACKEND_DX12)
#include "device_dx12.h"
#else
#error unsupported FSR2 backend
#endif


Device& Device::Instance()
{
#if defined(FSR_BACKEND_DX11)
    static DeviceDX11 instance;
#elif defined(FSR_BACKEND_DX12)
    static DeviceDX12 instance;
#else
#error unsupported FSR2 backend
#endif
    return instance;
}

bool Device::Init(IUnityInterfaces* unityInterfaces)
{
    if (!m_Initialized) {
        m_pUnityInterfaces = unityInterfaces;
        m_Initialized = InternalInit();
    }
    return m_Initialized;
}

void Device::Destroy()
{
    if (m_Initialized) {
        m_pUnityInterfaces = nullptr;
        m_Initialized = false;
        InternalDestroy();
    }
}
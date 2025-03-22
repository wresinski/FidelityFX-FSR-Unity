#include "device.h"

#include <memory>
#include <mutex>

#include "fsrunityplugin.h"

#if defined(FSR_BACKEND_DX11) || defined(FSR_BACKEND_ALL)
#include "device_dx11.h"
#endif
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
#include "device_dx12.h"
#endif


Device& Device::Instance(UnityGfxRenderer deviceType)
{
    static std::unique_ptr<Device> instance = nullptr;
    static std::mutex criticalSection;
    std::lock_guard<std::mutex> lock(criticalSection);
    switch (deviceType) {
#if defined(FSR_BACKEND_DX11) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D11:
        instance.reset(new DeviceDX11);
        break;
#endif
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
        instance.reset(new DeviceDX12);
        break;
#endif
    case kUnityGfxRendererNull:
        break;
    default:
        FSR_ERROR("Unsupported backend");
        break;
    }
    return *instance;
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
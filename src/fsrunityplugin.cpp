#include "fsrunityplugin.h"

#include <memory>

#include "IUnityRenderingExtensions.h"
#include "device.h"

#if defined(FSR_2)
#include "fsr2.h"
#elif defined(FSR_3)
#include "fsr3.h"
#elif defined(FSR_API)
#include "fsrapi.h"
#else
#error unknown FSR version
#endif


IUnityInterfaces* FSRUnityPlugin::UnityInterfaces = nullptr;
IUnityGraphics* FSRUnityPlugin::UnityGraphics = nullptr;
IUnityLog* FSRUnityPlugin::UnityLog = nullptr;

void UNITY_INTERFACE_API
OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    switch (eventType) {
    case kUnityGfxDeviceEventInitialize:
        Device::Instance(FSRUnityPlugin::UnityGraphics->GetRenderer()).Init(FSRUnityPlugin::UnityInterfaces);
        break;
    case kUnityGfxDeviceEventShutdown:
        Device::Instance().Destroy();
        break;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        FSRUnityPlugin::UnityInterfaces = unityInterfaces;
        FSRUnityPlugin::UnityLog = unityInterfaces->Get<IUnityLog>();
        FSRUnityPlugin::UnityGraphics = unityInterfaces->Get<IUnityGraphics>();
        FSRUnityPlugin::UnityGraphics->RegisterDeviceEventCallback(&OnGraphicsDeviceEvent);
        OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
    }

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
    {
        FSRUnityPlugin::UnityGraphics->UnregisterDeviceEventCallback(&OnGraphicsDeviceEvent);
    }

    uint32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FSRInit(
        uint32_t instanceID,
        const InitParam* initParam,
        uint32_t fsrVersion = 0)
    {
#if defined(FSR_API)
        return static_cast<uint32_t>(GetFSRInstance(instanceID).Init(*initParam, fsrVersion));
#else
        return static_cast<uint32_t>(GetFSRInstance(instanceID).Init(*initParam));
#endif
    }

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FSRGetProjectionMatrixJitterOffset(
        const int32_t index,
        const int32_t renderWidth,
        const int32_t displayWidth,
        float* outJitterOffset)
    {
        const auto& jitterOffset = GetFSRInstance(0).GetJitterOffset(index, renderWidth, displayWidth);
        if (outJitterOffset != nullptr) {
            outJitterOffset[0] = jitterOffset[0];
            outJitterOffset[1] = jitterOffset[1];
        }
    }

    uint32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FSRGenerateReactiveMask(
        uint32_t instanceID,
        const GenReactiveParam* genReactiveParam)
    {
        return static_cast<uint32_t>(GetFSRInstance(instanceID).GenerateReactiveMask(*genReactiveParam));
    }

    uint32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FSRDispatch(
        uint32_t instanceID,
        const DispatchParam* dispatchParam)
    {
        return static_cast<uint32_t>(GetFSRInstance(instanceID).Dispatch(*dispatchParam));
    }

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FSRDestroy(uint32_t instanceID)
    {
        GetFSRInstance(instanceID).Destroy();
    }

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FSRCallback(int eventID, void* data)
    {
        uint32_t instanceID = eventID >> 16;
        eventID &= 65535;
        if (data != nullptr) {
            switch ((FSRUnityPlugin::PassEvent)eventID) {
            case FSRUnityPlugin::PassEvent::INITIALIZE:
                FSRInit(instanceID, static_cast<InitParam*>(data));
                break;
            case FSRUnityPlugin::PassEvent::DISPATCH:
                FSRDispatch(instanceID, static_cast<DispatchParam*>(data));
                break;
            case FSRUnityPlugin::PassEvent::REACTIVEMASK:
                FSRGenerateReactiveMask(instanceID, static_cast<GenReactiveParam*>(data));
                break;
            case FSRUnityPlugin::PassEvent::DESTROY:
                FSRDestroy(instanceID);
                break;
            default:
                break;
            }
        } else
            FSR_ERROR("FSR Callback data is nullptr");
    }

    UnityRenderingEventAndData UNITY_INTERFACE_EXPORT  FSRGetCallback()
    {
        return FSRCallback;
    }

    void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FSRTextureUpdateCallback(UnityRenderingExtEventType eventType, void* data)
    {
        if (eventType == kUnityRenderingExtEventUpdateTextureBeginV2) {
            const UnityRenderingExtTextureUpdateParamsV2* params = reinterpret_cast<UnityRenderingExtTextureUpdateParamsV2*>(data);
            uint32_t userData = params->userData;
            uint32_t instanceID = userData >> 16;
            userData &= 65535;
            GetFSRInstance(instanceID).SetTextureID(static_cast<TextureName>(userData), static_cast<uint32_t>(params->textureID));
        }
    }

    typedef void (UNITY_INTERFACE_API* UnityTextureUpdateCallback)(UnityRenderingExtEventType, void*);

    UnityTextureUpdateCallback UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API FSRGetTextureUpdateCallback()
    {
        return FSRTextureUpdateCallback;
    }

#ifdef __cplusplus
}
#endif
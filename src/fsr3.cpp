#include "fsr3.h"

#include <memory>
#include <vector>

#include "fsrunityplugin.h"
#include "device.h"
#include "dllloader.h"

#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
#include "FidelityFX/host/backends/dx12/ffx_dx12.h"
#endif


size_t GetScratchMemorySize(size_t maxContexts);
FfxErrorCode GetInterface(void* device, void* scratchBuffer, size_t scratchBuffersize, FfxInterface* ffxInterface, uint32_t maxContexts);
FfxResource GetResource(FfxFsr3Context* context, void* res, const wchar_t* name = nullptr, FfxResourceStates state = FFX_RESOURCE_STATE_COMPUTE_READ);

FSR3& GetFSRInstance(uint32_t id)
{
    static std::vector<std::unique_ptr<FSR3> > instances;
    while (instances.size() <= id) {
        instances.emplace_back(new FSR3());
    }
    return *instances[id];
}

FfxErrorCode FSR3::Init(const InitParam& initParam)
{
    Destroy();
    m_Reset = true;
    FfxFsr3ContextDescription contextDesc{};
    contextDesc.flags = initParam.flags;
    contextDesc.flags |= FFX_FSR3_ENABLE_UPSCALING_ONLY;
    contextDesc.maxRenderSize.width = initParam.displaySizeWidth;
    contextDesc.maxRenderSize.height = initParam.displaySizeHeight;
    contextDesc.upscaleOutputSize.width = initParam.displaySizeWidth;
    contextDesc.upscaleOutputSize.height = initParam.displaySizeHeight;
    contextDesc.displaySize.width = initParam.displaySizeWidth;
    contextDesc.displaySize.height = initParam.displaySizeHeight;
    m_ScratchBuffer.resize(GetScratchMemorySize(FFX_FSR3_CONTEXT_COUNT));
    GetInterface(Device::Instance().GetNativeDevice(), m_ScratchBuffer.data(), m_ScratchBuffer.size(), &(contextDesc.backendInterfaceUpscaling), FFX_FSR3_CONTEXT_COUNT);

    const auto errorCode = ffxFsr3ContextCreate(&m_Context, &contextDesc);
    if (errorCode == FFX_OK) {
        m_ContextCreated = true;
    } else {
        FSR_ERROR("FFXFSR3 Init failed");
    }
    return errorCode;
}

void FSR3::Destroy()
{
    if (m_ContextCreated) {
        Device::Instance().Wait();
        ffxFsr3ContextDestroy(&m_Context);
        m_ContextCreated = false;
    }
}

std::array<float, 2> FSR3::GetJitterOffset(const int32_t index, const int32_t renderWidth, const int32_t displayWidth)
{
    std::array<float, 2> jitterOffset{};
    ffxFsr3GetJitterOffset(&(jitterOffset[0]), &(jitterOffset[1]), index, ffxFsr3GetJitterPhaseCount(renderWidth, displayWidth));
    return jitterOffset;
}

FfxErrorCode FSR3::GenerateReactiveMask(const GenReactiveParam& genReactiveParam)
{
    if (m_ContextCreated) {
        FfxCommandList commandList = Device::Instance().GetNativeCommandList();
        FfxFsr3GenerateReactiveDescription genReactiveDesc{};
        genReactiveDesc.commandList = commandList;
        genReactiveDesc.colorOpaqueOnly = GetResource(&m_Context, genReactiveParam.colorOpaqueOnly);
        //genReactiveDesc.colorOpaqueOnly = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::COLOR_OPAQUE_ONLY]));
        genReactiveDesc.colorPreUpscale = GetResource(&m_Context, genReactiveParam.colorPreUpscale);
        //genReactiveDesc.colorPreUpscale = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::COLOR_PRE_UPSCALE]));
        genReactiveDesc.outReactive = GetResource(&m_Context, genReactiveParam.outReactive, L"FSR3_InputReactiveMap", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        //genReactiveDesc.outReactive = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::REACTIVE]), L"FSR3_InputReactiveMap", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        genReactiveDesc.renderSize.width = genReactiveParam.renderSizeWidth;
        genReactiveDesc.renderSize.height = genReactiveParam.renderSizeHeight;
        genReactiveDesc.scale = genReactiveParam.scale;
        genReactiveDesc.cutoffThreshold = genReactiveParam.cutoffThreshold;
        genReactiveDesc.binaryValue = genReactiveParam.binaryValue;
        genReactiveDesc.flags = genReactiveParam.flags;
        const auto errorCode = ffxFsr3ContextGenerateReactiveMask(&m_Context, &genReactiveDesc);
        if (errorCode != FFX_OK) {
            FSR_ERROR("FFXFSR3 GenerateReactiveMask failed");
        }
        Device::Instance().ExecuteCommandList(commandList);
        return errorCode;
    } else
        return !FFX_OK;
}

FfxErrorCode FSR3::Dispatch(const DispatchParam& dispatchParam)
{
    if (m_ContextCreated) {
        FfxCommandList commandList = Device::Instance().GetNativeCommandList();
        FfxFsr3DispatchUpscaleDescription dispatchDesc{};
        dispatchDesc.commandList = commandList;
        dispatchDesc.color = GetResource(&m_Context, dispatchParam.color, L"FSR3_InputColor");
        //dispatchDesc.color = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::COLOR]), L"FSR3_InputColor");
        dispatchDesc.depth = GetResource(&m_Context, dispatchParam.depth, L"FSR3_InputDepth");
        //dispatchDesc.depth = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::DEPTH]), L"FSR3_InputDepth");
        dispatchDesc.motionVectors = GetResource(&m_Context, dispatchParam.motionVectors, L"FSR3_InputMotionVectors");
        //dispatchDesc.motionVectors = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::MOTION_VECTORS]), L"FSR3_InputMotionVectors");
        dispatchDesc.reactive = GetResource(&m_Context, dispatchParam.reactive, L"FSR3_InputReactiveMap");
        //dispatchDesc.reactive = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::REACTIVE]), L"FSR3_InputReactiveMap");
        dispatchDesc.transparencyAndComposition = GetResource(&m_Context, dispatchParam.transparencyAndComposition, L"FSR3_TransparencyAndCompositionMap");
        //dispatchDesc.transparencyAndComposition = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::TRANSPARENT_AND_COMPOSITION]), L"FSR3_TransparencyAndCompositionMap");
        dispatchDesc.upscaleOutput = GetResource(&m_Context, dispatchParam.output, L"FSR3_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        //dispatchDesc.upscaleOutput = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::OUTPUT]), L"FSR3_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        dispatchDesc.jitterOffset.x = dispatchParam.jitterOffsetX;
        dispatchDesc.jitterOffset.y = dispatchParam.jitterOffsetY;
        dispatchDesc.motionVectorScale.x = dispatchParam.motionVectorScaleX;
        dispatchDesc.motionVectorScale.y = dispatchParam.motionVectorScaleY;
        dispatchDesc.renderSize.width = dispatchParam.renderSizeWidth;
        dispatchDesc.renderSize.height = dispatchParam.renderSizeHeight;
        dispatchDesc.enableSharpening = dispatchParam.enableSharpening;
        dispatchDesc.sharpness = dispatchParam.sharpness;
        dispatchDesc.frameTimeDelta = dispatchParam.frameTimeDelta * 1000;
        dispatchDesc.preExposure = 1.0f;
        dispatchDesc.reset = m_Reset;
        dispatchDesc.cameraNear = dispatchParam.cameraNear;
        dispatchDesc.cameraFar = dispatchParam.cameraFar;
        dispatchDesc.cameraFovAngleVertical = dispatchParam.cameraFovAngleVertical;
        m_Reset = false;
        const auto errorCode = ffxFsr3ContextDispatchUpscale(&m_Context, &dispatchDesc);
        if (errorCode != FFX_OK) {
            FSR_ERROR("FFXFSR3 Dispatch failed");
        }
        Device::Instance().ExecuteCommandList(commandList);
        return errorCode;
    } else
        return !FFX_OK;
}

void FSR3::SetTextureID(const TextureName textureName, const UnityTextureID textureID)
{
    if (textureName > TextureName::INVALID && textureName < TextureName::MAX) {
        m_TextureIDs[textureName] = textureID;
    }
}

size_t GetScratchMemorySize(size_t maxContexts)
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
        return ffxGetScratchMemorySizeDX12(maxContexts);
#endif
    default:
        FSR_ERROR("Unsupported fsr3 backend");
        return 0;
    }
}

FfxErrorCode GetInterface(void* device, void* scratchBuffer, size_t scratchBuffersize, FfxInterface* ffxInterface, uint32_t maxContexts)
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
        return ffxGetInterfaceDX12(ffxInterface, static_cast<ID3D12Device*>(device), scratchBuffer, scratchBuffersize, maxContexts);
#endif
    default:
        FSR_ERROR("Unsupported fsr3 backend");
        return FFX_ERROR_NULL_DEVICE;
    }
}

FfxResource GetResource(FfxFsr3Context* context, void* res, const wchar_t* name, FfxResourceStates state)
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
    {
        auto getResourceState = [](uint32_t ffxState) {
            switch (ffxState) {
            case FFX_RESOURCE_STATE_UNORDERED_ACCESS:
                return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            case FFX_RESOURCE_STATE_COMPUTE_READ:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            case FFX_RESOURCE_STATE_COPY_SRC:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;
            case FFX_RESOURCE_STATE_COPY_DEST:
                return D3D12_RESOURCE_STATE_COPY_DEST;
            case FFX_RESOURCE_STATE_GENERIC_READ:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;
            default:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            }
        };
        Device::Instance().SetResourceState(res, getResourceState(state));
        return ffxGetResourceDX12(static_cast<ID3D12Resource*>(res), GetFfxResourceDescriptionDX12(static_cast<ID3D12Resource*>(res)), const_cast<wchar_t*>(name), state);
    }
#endif
    default:
        FSR_ERROR("Unsupported fsr3 backend");
        return FfxResource{};
    }
}

#if defined(FSR_BACKEND_ALL)
inline std::wstring GetDllName()
{
#if defined(_DEBUG)
    return L"ffx_fsr3_x64d.dll";
#else
    return L"ffx_fsr3_x64.dll";
#endif
}

inline std::wstring GetDllNameBackend()
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
    case kUnityGfxRendererD3D12:
#if defined(_DEBUG)
        return L"ffx_backend_dx12_x64d.dll";
#else
        return L"ffx_backend_dx12_x64.dll";
#endif
    default:
        FSR_ERROR("Unsupported fsr3 backend");
        return L"";
    }
}

typedef size_t(*PfnFfxGetScratchMemorySizeDX12)(size_t maxContexts);
size_t ffxGetScratchMemorySizeDX12(size_t maxContexts)
{
    static PfnFfxGetScratchMemorySizeDX12 getScratchMemorySizeDX12 = reinterpret_cast<PfnFfxGetScratchMemorySizeDX12>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxGetScratchMemorySizeDX12"));
    return getScratchMemorySizeDX12(maxContexts);
}

typedef FfxErrorCode (*PfnFfxGetInterfaceDX12)(FfxInterface* backendInterface, FfxDevice device, void* scratchBuffer, size_t scratchBufferSize, uint32_t maxContexts);
FfxErrorCode ffxGetInterfaceDX12(FfxInterface* backendInterface, FfxDevice device, void* scratchBuffer, size_t scratchBufferSize, uint32_t maxContexts)
{
    static PfnFfxGetInterfaceDX12 getInterfaceDX12 = reinterpret_cast<PfnFfxGetInterfaceDX12>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxGetInterfaceDX12"));
    return getInterfaceDX12(backendInterface, device, scratchBuffer, scratchBufferSize, maxContexts);
}

typedef FfxResource (*PfnFfxGetResourceDX12)(const ID3D12Resource* dx12Resource, FfxResourceDescription ffxResDescription, wchar_t* ffxResName, FfxResourceStates state);
FfxResource ffxGetResourceDX12(const ID3D12Resource* dx12Resource, FfxResourceDescription ffxResDescription, wchar_t* ffxResName, FfxResourceStates state)
{
    static PfnFfxGetResourceDX12 getResourceDX12 = reinterpret_cast<PfnFfxGetResourceDX12>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxGetResourceDX12"));
    return getResourceDX12(dx12Resource, ffxResDescription, ffxResName, state);
}

typedef FfxResourceDescription (*PfnGetFfxResourceDescriptionDX12)(ID3D12Resource* pResource);
FfxResourceDescription GetFfxResourceDescriptionDX12(ID3D12Resource* pResource)
{
    static PfnGetFfxResourceDescriptionDX12 getFfxResourceDescriptionDX12 = reinterpret_cast<PfnGetFfxResourceDescriptionDX12>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("GetFfxResourceDescriptionDX12"));
    return getFfxResourceDescriptionDX12(pResource);
}

typedef FfxErrorCode (*PfnFfxFsr3ContextCreate)(FfxFsr3Context* context, FfxFsr3ContextDescription* contextDescription);
FfxErrorCode ffxFsr3ContextCreate(FfxFsr3Context* context, FfxFsr3ContextDescription* contextDescription)
{
    static PfnFfxFsr3ContextCreate fsr3ContextCreate = reinterpret_cast<PfnFfxFsr3ContextCreate>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr3ContextCreate"));
    return fsr3ContextCreate(context, contextDescription);
}

typedef FfxErrorCode (*PfnFfxFsr3ContextDestroy)(FfxFsr3Context* context);
FfxErrorCode ffxFsr3ContextDestroy(FfxFsr3Context* context)
{
    static PfnFfxFsr3ContextDestroy fsr3ContextDestroy = reinterpret_cast<PfnFfxFsr3ContextDestroy>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr3ContextDestroy"));
    return fsr3ContextDestroy(context);
}

typedef FfxErrorCode (*PfnFfxFsr3GetJitterOffset)(float* outX, float* outY, int32_t index, int32_t phaseCount);
FfxErrorCode ffxFsr3GetJitterOffset(float* outX, float* outY, int32_t index, int32_t phaseCount)
{
    static PfnFfxFsr3GetJitterOffset fsr3GetJitterOffset = reinterpret_cast<PfnFfxFsr3GetJitterOffset>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr3GetJitterOffset"));
    return fsr3GetJitterOffset(outX, outY, index, phaseCount);
}

typedef int32_t (*PfnFfxFsr3GetJitterPhaseCount)(int32_t renderWidth, int32_t displayWidth);
int32_t ffxFsr3GetJitterPhaseCount(int32_t renderWidth, int32_t displayWidth)
{
    static PfnFfxFsr3GetJitterPhaseCount fsr3GetJitterPhaseCount = reinterpret_cast<PfnFfxFsr3GetJitterPhaseCount>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr3GetJitterPhaseCount"));
    return fsr3GetJitterPhaseCount(renderWidth, displayWidth);
}

typedef FfxErrorCode (*PfnFfxFsr3ContextGenerateReactiveMask)(FfxFsr3Context* context, const FfxFsr3GenerateReactiveDescription* params);
FfxErrorCode ffxFsr3ContextGenerateReactiveMask(FfxFsr3Context* context, const FfxFsr3GenerateReactiveDescription* params)
{
    static PfnFfxFsr3ContextGenerateReactiveMask fsr3ContextGenerateReactiveMask = reinterpret_cast<PfnFfxFsr3ContextGenerateReactiveMask>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr3ContextGenerateReactiveMask"));
    return fsr3ContextGenerateReactiveMask(context, params);
}

typedef FfxErrorCode (*PfnFfxFsr3ContextDispatchUpscale)(FfxFsr3Context* context, const FfxFsr3DispatchUpscaleDescription* dispatchParams);
FfxErrorCode ffxFsr3ContextDispatchUpscale(FfxFsr3Context* context, const FfxFsr3DispatchUpscaleDescription* dispatchParams)
{
    static PfnFfxFsr3ContextDispatchUpscale fsr3ContextDispatchUpscale = reinterpret_cast<PfnFfxFsr3ContextDispatchUpscale>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr3ContextDispatchUpscale"));
    return fsr3ContextDispatchUpscale(context, dispatchParams);
}
#endif
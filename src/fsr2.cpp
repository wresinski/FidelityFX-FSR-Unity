#include "fsr2.h"

#include <memory>
#include <vector>

#include "fsrunityplugin.h"
#include "device.h"
#include "dllloader.h"

#if defined(FSR_BACKEND_DX11) || defined(FSR_BACKEND_ALL)
#include "dx11/ffx_fsr2_dx11.h"
#endif
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
#include "dx12/ffx_fsr2_dx12.h"
#endif


size_t GetScratchMemorySize();
FfxErrorCode GetInterface(void* device, void* scratchBuffer, size_t scratchBuffersize, FfxFsr2Interface* fsr2Interface);
FfxResource GetResource(FfxFsr2Context* context, void* resource, const wchar_t* name = nullptr, FfxResourceStates state = FFX_RESOURCE_STATE_COMPUTE_READ);
FfxResource GetResourceByID(FfxFsr2Context* context, UnityTextureID textureID, const wchar_t* name = nullptr, FfxResourceStates state = FFX_RESOURCE_STATE_COMPUTE_READ);

FSR2& GetFSRInstance(uint32_t id)
{
    static std::vector<std::unique_ptr<FSR2> > instances;
    while (instances.size() <= id) {
        instances.emplace_back(new FSR2());
    }
    return *instances[id];
}

FfxErrorCode FSR2::Init(const InitParam& initParam)
{
    Destroy();
    m_Reset = true;
    FfxFsr2ContextDescription contextDesc{};
    contextDesc.flags = initParam.flags;
    contextDesc.maxRenderSize.width = initParam.displaySizeWidth;
    contextDesc.maxRenderSize.height = initParam.displaySizeHeight;
    contextDesc.displaySize.width = initParam.displaySizeWidth;
    contextDesc.displaySize.height = initParam.displaySizeHeight;
    contextDesc.device = Device::Instance().GetNativeDevice();
    m_ScratchBuffer.resize(GetScratchMemorySize());
    GetInterface(Device::Instance().GetNativeDevice(), m_ScratchBuffer.data(), m_ScratchBuffer.size(), &(contextDesc.callbacks));

    const auto errorCode = ffxFsr2ContextCreate(&m_Context, &contextDesc);
    if (errorCode == FFX_OK) {
        m_ContextCreated = true;
    } else {
        FSR_ERROR("FFXFSR2 Init failed");
    }
    return errorCode;
}

void FSR2::Destroy()
{
    if (m_ContextCreated) {
        Device::Instance().Wait();
        ffxFsr2ContextDestroy(&m_Context);
        m_ContextCreated = false;
    }
}

std::array<float, 2> FSR2::GetJitterOffset(const int32_t index, const int32_t renderWidth, const int32_t displayWidth)
{
    std::array<float, 2> jitterOffset{};
    ffxFsr2GetJitterOffset(&(jitterOffset[0]), &(jitterOffset[1]), index, ffxFsr2GetJitterPhaseCount(renderWidth, displayWidth));
    return jitterOffset;
}

FfxErrorCode FSR2::GenerateReactiveMask(const GenReactiveParam& genReactiveParam)
{
    if (m_ContextCreated) {
        FfxCommandList commandList = Device::Instance().GetNativeCommandList();
        FfxFsr2GenerateReactiveDescription genReactiveDesc{};
        genReactiveDesc.commandList = commandList;
        genReactiveDesc.colorOpaqueOnly = GetResource(&m_Context, genReactiveParam.colorOpaqueOnly);
        //genReactiveDesc.colorOpaqueOnly = GetResourceByID(&m_Context, m_TextureIDs[TextureName::COLOR_OPAQUE_ONLY]);
        genReactiveDesc.colorPreUpscale = GetResource(&m_Context, genReactiveParam.colorPreUpscale);
        //genReactiveDesc.colorPreUpscale = GetResourceByID(&m_Context, m_TextureIDs[TextureName::COLOR_PRE_UPSCALE]);
        genReactiveDesc.outReactive = GetResource(&m_Context, genReactiveParam.outReactive, L"FSR2_InputReactiveMap", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        //genReactiveDesc.outReactive = GetResourceByID(&m_Context, m_TextureIDs[TextureName::REACTIVE], L"FSR2_InputReactiveMap", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        genReactiveDesc.renderSize.width = genReactiveParam.renderSizeWidth;
        genReactiveDesc.renderSize.height = genReactiveParam.renderSizeHeight;
        genReactiveDesc.scale = genReactiveParam.scale;
        genReactiveDesc.cutoffThreshold = genReactiveParam.cutoffThreshold;
        genReactiveDesc.binaryValue = genReactiveParam.binaryValue;
        genReactiveDesc.flags = genReactiveParam.flags;
        const auto err = ffxFsr2ContextGenerateReactiveMask(&m_Context, &genReactiveDesc);
        if (err != FFX_OK) {
            FSR_ERROR("FFXFSR2 GenerateReactiveMask failed");
        }
        Device::Instance().ExecuteCommandList(commandList);
        return err;
    } else
        return !FFX_OK;
}

FfxErrorCode FSR2::Dispatch(const DispatchParam& dispatchParam)
{
    if (m_ContextCreated) {
        FfxCommandList commandList = Device::Instance().GetNativeCommandList();
        FfxFsr2DispatchDescription dispatchDesc{};
        dispatchDesc.commandList = commandList;
        dispatchDesc.color = GetResource(&m_Context, dispatchParam.color, L"FSR2_InputColor");
        //dispatchDesc.color = GetResourceByID(&m_Context, m_TextureIDs[TextureName::COLOR], L"FSR2_InputColor");
        dispatchDesc.depth = GetResource(&m_Context, dispatchParam.depth, L"FSR2_InputDepth");
        //dispatchDesc.depth = GetResourceByID(&m_Context, m_TextureIDs[TextureName::DEPTH], L"FSR2_InputDepth");
        dispatchDesc.motionVectors = GetResource(&m_Context, dispatchParam.motionVectors, L"FSR2_InputMotionVectors");
        //dispatchDesc.motionVectors = GetResourceByID(&m_Context, m_TextureIDs[TextureName::MOTION_VECTORS], L"FSR2_InputMotionVectors");
        dispatchDesc.reactive = GetResource(&m_Context, dispatchParam.reactive, L"FSR2_InputReactiveMap");
        //dispatchDesc.reactive = GetResourceByID(&m_Context, m_TextureIDs[TextureName::REACTIVE], L"FSR2_InputReactiveMap");
        dispatchDesc.transparencyAndComposition = GetResource(&m_Context, dispatchParam.transparencyAndComposition, L"FSR2_TransparencyAndCompositionMap");
        //dispatchDesc.transparencyAndComposition = GetResourceByID(&m_Context, m_TextureIDs[TextureName::TRANSPARENT_AND_COMPOSITION], L"FSR2_TransparencyAndCompositionMap");
        dispatchDesc.output = GetResource(&m_Context, dispatchParam.output, L"FSR2_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        //dispatchDesc.output = GetResourceByID(&m_Context, m_TextureIDs[TextureName::OUTPUT], L"FSR2_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
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
        const auto err = ffxFsr2ContextDispatch(&m_Context, &dispatchDesc);
        if (err != FFX_OK) {
            FSR_ERROR("FFXFSR2 Dispatch failed");
        }
        Device::Instance().ExecuteCommandList(commandList);
        return err;
    } else
        return !FFX_OK;
}

void FSR2::SetTextureID(const TextureName textureName, const UnityTextureID textureID)
{
    if (textureName > TextureName::INVALID && textureName < TextureName::MAX) {
        m_TextureIDs[textureName] = textureID;
    }
}

size_t GetScratchMemorySize()
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX11) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D11:
        return ffxFsr2GetScratchMemorySizeDX11();
#endif
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
        return ffxFsr2GetScratchMemorySizeDX12();
#endif
    default:
        FSR_ERROR("Unsupported fsr2 backend");
        return 0;
    }
}

FfxErrorCode GetInterface(void* device, void* scratchBuffer, size_t scratchBuffersize, FfxFsr2Interface* fsr2Interface)
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX11) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D11:
        return ffxFsr2GetInterfaceDX11(fsr2Interface, static_cast<ID3D11Device*>(device), scratchBuffer, scratchBuffersize);
#endif
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
        return ffxFsr2GetInterfaceDX12(fsr2Interface, static_cast<ID3D12Device*>(device), scratchBuffer, scratchBuffersize);
#endif
    default:
        FSR_ERROR("Unsupported fsr2 backend");
        return FFX_ERROR_NULL_DEVICE;
    }
}

FfxResource GetResource(FfxFsr2Context* context, void* resource, const wchar_t* name, FfxResourceStates state)
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX11) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D11:
    {
        void* nativeResource = Device::Instance().GetNativeResource(resource);
        return ffxGetResourceDX11(context, static_cast<ID3D11Resource*>(nativeResource), name, state);
    }
#endif
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
        void* nativeResource = Device::Instance().GetNativeResource(resource, nullptr, getResourceState(state));
        return ffxGetResourceDX12(context, static_cast<ID3D12Resource*>(nativeResource), name, state);
    }
#endif
    default:
        FSR_ERROR("Unsupported fsr2 backend");
        return FfxResource{};
    }
}

FfxResource GetResourceByID(FfxFsr2Context* context, UnityTextureID textureID, const wchar_t* name, FfxResourceStates state)
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX11) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D11:
    {
        void* nativeResource = Device::Instance().GetNativeResourceByID(textureID);
        return ffxGetResourceDX11(context, static_cast<ID3D11Resource*>(nativeResource), name, state);
    }
#endif
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
        void* nativeResource = Device::Instance().GetNativeResourceByID(textureID, nullptr, getResourceState(state));
        return ffxGetResourceDX12(context, static_cast<ID3D12Resource*>(nativeResource), name, state);
    }
#endif
    default:
        FSR_ERROR("Unsupported fsr2 backend");
        return FfxResource{};
    }
}

#if defined(FSR_BACKEND_ALL)
inline std::wstring GetDllName()
{
#if defined(_DEBUG)
    return L"ffx_fsr2_api_x64d.dll";
#else
    return L"ffx_fsr2_api_x64.dll";
#endif
}

inline std::wstring GetDllNameBackend()
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
    case kUnityGfxRendererD3D11:
#if defined(_DEBUG)
        return L"ffx_fsr2_api_dx11_x64d.dll";
#else
        return L"ffx_fsr2_api_dx11_x64.dll";
#endif
    case kUnityGfxRendererD3D12:
#if defined(_DEBUG)
        return L"ffx_fsr2_api_dx12_x64d.dll";
#else
        return L"ffx_fsr2_api_dx12_x64.dll";
#endif
    default:
        FSR_ERROR("Unsupported fsr2 backend");
        return L"";
    }
}

typedef size_t(*PfnFfxFsr2GetScratchMemorySizeDX11)();
size_t ffxFsr2GetScratchMemorySizeDX11()
{
    static PfnFfxFsr2GetScratchMemorySizeDX11 getScratchMemorySizeDX11 = reinterpret_cast<PfnFfxFsr2GetScratchMemorySizeDX11>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxFsr2GetScratchMemorySizeDX11"));
    return getScratchMemorySizeDX11();
}

typedef size_t(*PfnFfxFsr2GetScratchMemorySizeDX12)();
size_t ffxFsr2GetScratchMemorySizeDX12()
{
    static PfnFfxFsr2GetScratchMemorySizeDX12 getScratchMemorySizeDX12 = reinterpret_cast<PfnFfxFsr2GetScratchMemorySizeDX12>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxFsr2GetScratchMemorySizeDX12"));
    return getScratchMemorySizeDX12();
}

typedef FfxErrorCode(*PfnFfxFsr2GetInterfaceDX11)(FfxFsr2Interface* fsr2Interface, ID3D11Device* device, void* scratchBuffer, size_t scratchBufferSize);
FfxErrorCode ffxFsr2GetInterfaceDX11(FfxFsr2Interface* fsr2Interface, ID3D11Device* device, void* scratchBuffer, size_t scratchBufferSize)
{
    static PfnFfxFsr2GetInterfaceDX11 getInterfaceDX11 = reinterpret_cast<PfnFfxFsr2GetInterfaceDX11>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxFsr2GetInterfaceDX11"));
    return getInterfaceDX11(fsr2Interface, device, scratchBuffer, scratchBufferSize);
}

typedef FfxErrorCode(*PfnFfxFsr2GetInterfaceDX12)(FfxFsr2Interface* fsr2Interface, ID3D12Device* device, void* scratchBuffer, size_t scratchBufferSize);
FfxErrorCode ffxFsr2GetInterfaceDX12(FfxFsr2Interface* fsr2Interface, ID3D12Device* device, void* scratchBuffer, size_t scratchBufferSize)
{
    static PfnFfxFsr2GetInterfaceDX12 getInterfaceDX12 = reinterpret_cast<PfnFfxFsr2GetInterfaceDX12>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxFsr2GetInterfaceDX12"));
    return getInterfaceDX12(fsr2Interface, device, scratchBuffer, scratchBufferSize);
}

typedef FfxResource(*PfnFfxGetResourceDX11)(FfxFsr2Context* context, ID3D11Resource* resDx11, const wchar_t* name, FfxResourceStates state);
FfxResource ffxGetResourceDX11(FfxFsr2Context* context, ID3D11Resource* resDx11, const wchar_t* name, FfxResourceStates state)
{
    static PfnFfxGetResourceDX11 getResourceDX11 = reinterpret_cast<PfnFfxGetResourceDX11>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxGetResourceDX11"));
    return getResourceDX11(context, resDx11, name, state);
}

typedef FfxResource(*PfnFfxGetResourceDX12)(FfxFsr2Context* context, ID3D12Resource* resDx12, const wchar_t* name, FfxResourceStates state, UINT shaderComponentMapping);
FfxResource ffxGetResourceDX12(FfxFsr2Context* context, ID3D12Resource* resDx12, const wchar_t* name, FfxResourceStates state, UINT shaderComponentMapping)
{
    static PfnFfxGetResourceDX12 getResourceDX12 = reinterpret_cast<PfnFfxGetResourceDX12>(DllLoader::Instance(GetDllNameBackend().c_str()).GetProcAddress("ffxGetResourceDX12"));
    return getResourceDX12(context, resDx12, name, state, shaderComponentMapping);
}

typedef FfxErrorCode(*PfnFfxFsr2ContextCreate)(FfxFsr2Context* context, const FfxFsr2ContextDescription* contextDescription);
FfxErrorCode ffxFsr2ContextCreate(FfxFsr2Context* context, const FfxFsr2ContextDescription* contextDescription)
{
    static PfnFfxFsr2ContextCreate fsr2ContextCreate = reinterpret_cast<PfnFfxFsr2ContextCreate>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr2ContextCreate"));
    return fsr2ContextCreate(context, contextDescription);
}

typedef FfxErrorCode(*PfnFfxFsr2ContextDestroy)(FfxFsr2Context* context);
FfxErrorCode ffxFsr2ContextDestroy(FfxFsr2Context* context)
{
    static PfnFfxFsr2ContextDestroy fsr2ContextDestroy = reinterpret_cast<PfnFfxFsr2ContextDestroy>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr2ContextDestroy"));
    return fsr2ContextDestroy(context);
}

typedef FfxErrorCode(*PfnFfxFsr2GetJitterOffset)(float* outX, float* outY, int32_t index, int32_t phaseCount);
FfxErrorCode ffxFsr2GetJitterOffset(float* outX, float* outY, int32_t index, int32_t phaseCount)
{
    static PfnFfxFsr2GetJitterOffset fsr2GetJitterOffset = reinterpret_cast<PfnFfxFsr2GetJitterOffset>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr2GetJitterOffset"));
    return fsr2GetJitterOffset(outX, outY, index, phaseCount);
}

typedef int32_t(*PfnFfxFsr2GetJitterPhaseCount)(int32_t renderWidth, int32_t displayWidth);
int32_t ffxFsr2GetJitterPhaseCount(int32_t renderWidth, int32_t displayWidth)
{
    static PfnFfxFsr2GetJitterPhaseCount fsr2GetJitterPhaseCount = reinterpret_cast<PfnFfxFsr2GetJitterPhaseCount>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr2GetJitterPhaseCount"));
    return fsr2GetJitterPhaseCount(renderWidth, displayWidth);
}

typedef FfxErrorCode(*PfnFfxFsr2ContextGenerateReactiveMask)(FfxFsr2Context* context, const FfxFsr2GenerateReactiveDescription* params);
FfxErrorCode ffxFsr2ContextGenerateReactiveMask(FfxFsr2Context* context, const FfxFsr2GenerateReactiveDescription* params)
{
    static PfnFfxFsr2ContextGenerateReactiveMask fsr2ContextGenerateReactiveMask = reinterpret_cast<PfnFfxFsr2ContextGenerateReactiveMask>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr2ContextGenerateReactiveMask"));
    return fsr2ContextGenerateReactiveMask(context, params);
}

typedef FfxErrorCode(*PfnFfxFsr2ContextDispatch)(FfxFsr2Context* context, const FfxFsr2DispatchDescription* dispatchDescription);
FfxErrorCode ffxFsr2ContextDispatch(FfxFsr2Context* context, const FfxFsr2DispatchDescription* dispatchDescription)
{
    static PfnFfxFsr2ContextDispatch fsr2ContextDispatch = reinterpret_cast<PfnFfxFsr2ContextDispatch>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxFsr2ContextDispatch"));
    return fsr2ContextDispatch(context, dispatchDescription);
}
#endif
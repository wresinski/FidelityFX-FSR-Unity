#include "fsr3.h"

#include <memory>
#include <vector>

#include "fsrunityplugin.h"
#include "device.h"

#if defined(FSR_BACKEND_DX12)
#include "FidelityFX/host/backends/dx12/ffx_dx12.h"
#else
#error unsupported FSR3 backend
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

FSR3::~FSR3()
{
    Destroy();
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
#if defined(FSR_BACKEND_DX12)
    return ffxGetScratchMemorySizeDX12(maxContexts);
#else
#error unsupported FSR3 backend
#endif
}

FfxErrorCode GetInterface(void* device, void* scratchBuffer, size_t scratchBuffersize, FfxInterface* ffxInterface, uint32_t maxContexts)
{
#if defined(FSR_BACKEND_DX12)
    return ffxGetInterfaceDX12(ffxInterface, static_cast<ID3D12Device*>(device), scratchBuffer, scratchBuffersize, maxContexts);
#else
#error unsupported FSR3 backend
#endif
}

FfxResource GetResource(FfxFsr3Context* context, void* res, const wchar_t* name, FfxResourceStates state)
{
#if defined(FSR_BACKEND_DX12)
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
#else
#error unsupported FSR3 backend
#endif
}
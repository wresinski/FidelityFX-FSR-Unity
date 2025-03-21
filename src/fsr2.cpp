#include "fsr2.h"

#include <memory>
#include <vector>

#include "fsrunityplugin.h"
#include "device.h"

#if defined(FSR_BACKEND_DX11)
#include "dx11/ffx_fsr2_dx11.h"
#elif defined(FSR_BACKEND_DX12)
#include "dx12/ffx_fsr2_dx12.h"
#else
#error unsupported FSR2 backend
#endif


size_t GetScratchMemorySize();
FfxErrorCode GetInterface(void* device, void* scratchBuffer, size_t scratchBuffersize, FfxFsr2Interface* fsr2Interface);
FfxResource GetResource(FfxFsr2Context* context, void* res, const wchar_t* name = nullptr, FfxResourceStates state = FFX_RESOURCE_STATE_COMPUTE_READ);

FSR2& GetFSRInstance(uint32_t id)
{
    static std::vector<std::unique_ptr<FSR2> > instances;
    while (instances.size() <= id) {
        instances.emplace_back(new FSR2());
    }
    return *instances[id];
}

FSR2::~FSR2()
{
    Destroy();
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
        //genReactiveDesc.colorOpaqueOnly = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::COLOR_OPAQUE_ONLY]));
        genReactiveDesc.colorPreUpscale = GetResource(&m_Context, genReactiveParam.colorPreUpscale);
        //genReactiveDesc.colorPreUpscale = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::COLOR_PRE_UPSCALE]));
        genReactiveDesc.outReactive = GetResource(&m_Context, genReactiveParam.outReactive, L"FSR2_InputReactiveMap", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        //genReactiveDesc.outReactive = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::REACTIVE]), L"FSR2_InputReactiveMap", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
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
        //dispatchDesc.color = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::COLOR]), L"FSR2_InputColor");
        dispatchDesc.depth = GetResource(&m_Context, dispatchParam.depth, L"FSR2_InputDepth");
        //dispatchDesc.depth = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::DEPTH]), L"FSR2_InputDepth");
        dispatchDesc.motionVectors = GetResource(&m_Context, dispatchParam.motionVectors, L"FSR2_InputMotionVectors");
        //dispatchDesc.motionVectors = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::MOTION_VECTORS]), L"FSR2_InputMotionVectors");
        dispatchDesc.reactive = GetResource(&m_Context, dispatchParam.reactive, L"FSR2_InputReactiveMap");
        //dispatchDesc.reactive = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::REACTIVE]), L"FSR2_InputReactiveMap");
        dispatchDesc.transparencyAndComposition = GetResource(&m_Context, dispatchParam.transparencyAndComposition, L"FSR2_TransparencyAndCompositionMap");
        //dispatchDesc.transparencyAndComposition = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::TRANSPARENT_AND_COMPOSITION]), L"FSR2_TransparencyAndCompositionMap");
        dispatchDesc.output = GetResource(&m_Context, dispatchParam.output, L"FSR2_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
        //dispatchDesc.output = GetResource(&m_Context, Device::Instance().GetNativeResource(m_TextureIDs[TextureName::OUTPUT]), L"FSR2_OutputUpscaledColor", FFX_RESOURCE_STATE_UNORDERED_ACCESS);
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
#if defined(FSR_BACKEND_DX11)
    return ffxFsr2GetScratchMemorySizeDX11();
#elif defined(FSR_BACKEND_DX12)
    return ffxFsr2GetScratchMemorySizeDX12();
#else
#error unsupported FSR2 backend
#endif
}

FfxErrorCode GetInterface(void* device, void* scratchBuffer, size_t scratchBuffersize, FfxFsr2Interface* fsr2Interface)
{
#if defined(FSR_BACKEND_DX11)
    return ffxFsr2GetInterfaceDX11(fsr2Interface, static_cast<ID3D11Device*>(device), scratchBuffer, scratchBuffersize);
#elif defined(FSR_BACKEND_DX12)
    return ffxFsr2GetInterfaceDX12(fsr2Interface, static_cast<ID3D12Device*>(device), scratchBuffer, scratchBuffersize);
#else
#error unsupported FSR2 backend
#endif
}

FfxResource GetResource(FfxFsr2Context* context, void* res, const wchar_t* name, FfxResourceStates state)
{
#if defined(FSR_BACKEND_DX11)
    return ffxGetResourceDX11(context, static_cast<ID3D11Resource*>(res), name, state);
#elif defined(FSR_BACKEND_DX12)
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

    return ffxGetResourceDX12(context, static_cast<ID3D12Resource*>(res), name, state);
#else
#error unsupported FSR2 backend
#endif
}
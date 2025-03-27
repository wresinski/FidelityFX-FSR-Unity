#include "fsrapi.h"

#include <memory>
#include <vector>

#include "fsrunityplugin.h"
#include "device.h"
#include "dllloader.h"

#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
#include "dx12/ffx_api_dx12.hpp"
#endif


FfxApiResource ffxApiGetResource(void* resource, uint32_t state = FFX_API_RESOURCE_STATE_COMPUTE_READ, uint32_t additionalUsages = 0);
FfxApiResource ffxApiGetResourceByID(UnityTextureID textureID, uint32_t state = FFX_API_RESOURCE_STATE_COMPUTE_READ, uint32_t additionalUsages = 0);

FSRAPI& GetFSRInstance(uint32_t id)
{
    static std::vector<std::unique_ptr<FSRAPI> > instances;
    while (instances.size() <= id) {
        instances.emplace_back(new FSRAPI());
    }
    return *instances[id];
}

uint64_t FSRAPI::Query(uint32_t fsrVersion)
{
    uint64_t versionId = 0;
    ffx::QueryDescGetVersions versionQuery{};
    versionQuery.createDescType = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    if (Device::Instance().GetDeviceType() == kUnityGfxRendererD3D12) {
        versionQuery.device = Device::Instance().GetNativeDevice();
    }
    uint64_t versionCount = 0;
    versionQuery.outputCount = &versionCount;
    ffxQuery(nullptr, &versionQuery.header);

    std::vector<const char*> versionNames;
    std::vector<uint64_t> fsrVersionIds;
    fsrVersionIds.resize(versionCount);
    versionNames.resize(versionCount);
    versionQuery.versionIds = fsrVersionIds.data();
    versionQuery.versionNames = versionNames.data();
    ffxQuery(nullptr, &versionQuery.header);

    for (size_t i = 0; i < versionCount; ++i) {
        if (static_cast<uint32_t>(versionNames[i][0] - '0') == fsrVersion) {
            versionId = fsrVersionIds[i];
            break;
        }
    }
    return versionId;
}

ffx::ReturnCode FSRAPI::Init(const InitParam& initParam, uint32_t fsrVersion)
{
    Destroy();

    // get version info from ffxapi
    ffx::CreateContextDescOverrideVersion versionOverride{};
    if (fsrVersion != 0) {
        versionOverride.versionId = Query(fsrVersion);
        if (versionOverride.versionId == 0) {
            return ffx::ReturnCode::ErrorNoProvider;
        }
    }

    m_Reset = true;

    ffx::CreateContextDescUpscale createFsr{};
    createFsr.maxUpscaleSize = {initParam.displaySizeWidth, initParam.displaySizeHeight};
    createFsr.maxRenderSize = {initParam.displaySizeWidth, initParam.displaySizeHeight};
    createFsr.flags = initParam.flags;

    ffx::ReturnCode retCode = ffx::ReturnCode::Error;
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
    {
        ffx::CreateBackendDX12Desc backendDesc{};
        backendDesc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12;
        backendDesc.device = static_cast<ID3D12Device*>(Device::Instance().GetNativeDevice());
        if (fsrVersion != 0) {
            retCode = ffx::CreateContext(m_Context, nullptr, createFsr, backendDesc, versionOverride);
        } else {
            retCode = ffx::CreateContext(m_Context, nullptr, createFsr, backendDesc);
        }
        break;
    }
#endif
    default:
        FSR_ERROR("Unsupported fsrapi backend");
        break;
    }

    if (retCode == ffx::ReturnCode::Ok) {
        m_ContextCreated = true;
    } else {
        FSR_ERROR("ffxCreateContext Init failed");
    }
    return retCode;
}

void FSRAPI::Destroy()
{
    if (m_ContextCreated) {
        Device::Instance().Wait();
        ffx::DestroyContext(m_Context);
        m_ContextCreated = false;
    }
}

std::array<float, 2> FSRAPI::GetJitterOffset(const int32_t index, const int32_t renderWidth, const int32_t displayWidth)
{
    std::array<float, 2> jitterOffset{};
    if (m_ContextCreated) {
        ffx::ReturnCode retCode;
        int32_t jitterPhaseCount;
        ffx::QueryDescUpscaleGetJitterPhaseCount getJitterPhaseDesc{};
        getJitterPhaseDesc.displayWidth = renderWidth;
        getJitterPhaseDesc.renderWidth = displayWidth;
        getJitterPhaseDesc.pOutPhaseCount = &jitterPhaseCount;

        retCode = ffx::Query(m_Context, getJitterPhaseDesc);
        if (retCode != ffx::ReturnCode::Ok) {
            FSR_ERROR("ffxQuery GetJitterPhaseCount failed");
        }
        ffx::QueryDescUpscaleGetJitterOffset getJitterOffsetDesc{};
        getJitterOffsetDesc.index = index;
        getJitterOffsetDesc.phaseCount = jitterPhaseCount;
        getJitterOffsetDesc.pOutX = &jitterOffset[0];
        getJitterOffsetDesc.pOutY = &jitterOffset[1];

        retCode = ffx::Query(m_Context, getJitterOffsetDesc);
    }
    return jitterOffset;
}

ffx::ReturnCode FSRAPI::GenerateReactiveMask(const GenReactiveParam& genReactiveParam)
{
    if (m_ContextCreated) {
        void* commandList = Device::Instance().GetNativeCommandList();

        ffx::DispatchDescUpscaleGenerateReactiveMask genReactiveDesc{};
        genReactiveDesc.commandList = commandList;
        genReactiveDesc.colorOpaqueOnly = ffxApiGetResource(genReactiveParam.colorOpaqueOnly);
        //genReactiveDesc.colorOpaqueOnly = ffxApiGetResourceByID(m_TextureIDs[TextureName::COLOR_OPAQUE_ONLY]);
        genReactiveDesc.colorPreUpscale = ffxApiGetResource(genReactiveParam.colorPreUpscale);
        //genReactiveDesc.colorPreUpscale = ffxApiGetResourceByID(m_TextureIDs[TextureName::COLOR_PRE_UPSCALE]);
        genReactiveDesc.outReactive = ffxApiGetResource(genReactiveParam.outReactive, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        //genReactiveDesc.outReactive = ffxApiGetResourceByID(m_TextureIDs[TextureName::REACTIVE], FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        genReactiveDesc.renderSize.width = genReactiveParam.renderSizeWidth;
        genReactiveDesc.renderSize.height = genReactiveParam.renderSizeHeight;
        genReactiveDesc.scale = genReactiveParam.scale;
        genReactiveDesc.cutoffThreshold = genReactiveParam.cutoffThreshold;
        genReactiveDesc.binaryValue = genReactiveParam.binaryValue;
        genReactiveDesc.flags = genReactiveParam.flags;
        ffx::ReturnCode retCode = ffx::Dispatch(m_Context, genReactiveDesc);
        if (retCode != ffx::ReturnCode::Ok) {
            FSR_ERROR("ffxDispatch GenerateReactiveMask failed");
        }
        Device::Instance().ExecuteCommandList(commandList);
        return retCode;
    } else
        return ffx::ReturnCode::Error;
}

ffx::ReturnCode FSRAPI::Dispatch(const DispatchParam& dispatchParam)
{
    if (m_ContextCreated) {
        void* commandList = Device::Instance().GetNativeCommandList();

        ffx::DispatchDescUpscale dispatchDesc{};
        dispatchDesc.commandList = commandList;
        dispatchDesc.color = ffxApiGetResource(dispatchParam.color);
        //dispatchDesc.color = ffxApiGetResourceByID(m_TextureIDs[TextureName::COLOR]);
        dispatchDesc.depth = ffxApiGetResource(dispatchParam.depth);
        //dispatchDesc.depth = ffxApiGetResourceByID(m_TextureIDs[TextureName::DEPTH]);
        dispatchDesc.motionVectors = ffxApiGetResource(dispatchParam.motionVectors);
        //dispatchDesc.motionVectors = ffxApiGetResourceByID(m_TextureIDs[TextureName::MOTION_VECTORS]);
        dispatchDesc.reactive = ffxApiGetResource(dispatchParam.reactive);
        //dispatchDesc.reactive = ffxApiGetResourceByID(m_TextureIDs[TextureName::REACTIVE]);
        dispatchDesc.transparencyAndComposition = ffxApiGetResource(dispatchParam.transparencyAndComposition);
        //dispatchDesc.transparencyAndComposition = ffxApiGetResourceByID(m_TextureIDs[TextureName::TRANSPARENT_AND_COMPOSITION]);
        dispatchDesc.output = ffxApiGetResource(dispatchParam.output, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        //dispatchDesc.output = ffxApiGetResourceByID(m_TextureIDs[TextureName::OUTPUT], FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        dispatchDesc.jitterOffset.x = dispatchParam.jitterOffsetX;
        dispatchDesc.jitterOffset.y = dispatchParam.jitterOffsetY;
        dispatchDesc.motionVectorScale.x = dispatchParam.motionVectorScaleX;
        dispatchDesc.motionVectorScale.y = dispatchParam.motionVectorScaleY;
        dispatchDesc.reset = m_Reset;
        dispatchDesc.enableSharpening = dispatchParam.enableSharpening;
        dispatchDesc.sharpness = dispatchParam.sharpness;
        dispatchDesc.frameTimeDelta = dispatchParam.frameTimeDelta * 1000;
        dispatchDesc.preExposure = 1.0f;
        dispatchDesc.renderSize.width = dispatchParam.renderSizeWidth;
        dispatchDesc.renderSize.height = dispatchParam.renderSizeHeight;
        dispatchDesc.cameraFovAngleVertical = dispatchParam.cameraFovAngleVertical;
        dispatchDesc.cameraFar = dispatchParam.cameraFar;
        dispatchDesc.cameraNear = dispatchParam.cameraNear;
        dispatchDesc.flags = 0;

        m_Reset = false;
        ffx::ReturnCode retCode = ffx::Dispatch(m_Context, dispatchDesc);
        if (retCode != ffx::ReturnCode::Ok) {
            FSR_ERROR("ffxDispatch Dispatch failed");
        }
        Device::Instance().ExecuteCommandList(commandList);
        return retCode;
    } else
        return ffx::ReturnCode::Error;
}

void FSRAPI::SetTextureID(const TextureName textureName, const UnityTextureID textureID)
{
    if (textureName > TextureName::INVALID && textureName < TextureName::MAX) {
        m_TextureIDs[textureName] = textureID;
    }
}

FfxApiResource ffxApiGetResource(void* resource, uint32_t state, uint32_t additionalUsages)
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
    {
        auto getResourceState = [](uint32_t ffxState) {
            switch (ffxState) {
            case FFX_API_RESOURCE_STATE_UNORDERED_ACCESS:
                return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            case FFX_API_RESOURCE_STATE_COMPUTE_READ:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            case FFX_API_RESOURCE_STATE_COPY_SRC:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;
            case FFX_API_RESOURCE_STATE_COPY_DEST:
                return D3D12_RESOURCE_STATE_COPY_DEST;
            case FFX_API_RESOURCE_STATE_GENERIC_READ:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;
            default:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            }
        };
        void* nativeResource = Device::Instance().GetNativeResource(resource, nullptr, getResourceState(state));
        return ffxApiGetResourceDX12(static_cast<ID3D12Resource*>(nativeResource), state);
    }
#endif
    default:
        FSR_ERROR("Unsupported fsrapi backend");
        return FfxApiResource{};
    }
}

FfxApiResource ffxApiGetResourceByID(UnityTextureID textureID, uint32_t state, uint32_t additionalUsages)
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
    case kUnityGfxRendererD3D12:
    {
        auto getResourceState = [](uint32_t ffxState) {
            switch (ffxState) {
            case FFX_API_RESOURCE_STATE_UNORDERED_ACCESS:
                return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            case FFX_API_RESOURCE_STATE_COMPUTE_READ:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            case FFX_API_RESOURCE_STATE_COPY_SRC:
                return D3D12_RESOURCE_STATE_COPY_SOURCE;
            case FFX_API_RESOURCE_STATE_COPY_DEST:
                return D3D12_RESOURCE_STATE_COPY_DEST;
            case FFX_API_RESOURCE_STATE_GENERIC_READ:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_COPY_SOURCE;
            default:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            }
        };
        void* nativeResource = Device::Instance().GetNativeResourceByID(textureID, nullptr, getResourceState(state));
        return ffxApiGetResourceDX12(static_cast<ID3D12Resource*>(nativeResource), state);
    }
#endif
    default:
        FSR_ERROR("Unsupported fsrapi backend");
        return FfxApiResource{};
    }
}

#if defined(FSR_BACKEND_ALL)
inline std::wstring GetDllName()
{
    UnityGfxRenderer renderer = Device::Instance().GetDeviceType();
    switch (renderer) {
    case kUnityGfxRendererD3D12:
#if defined(_DEBUG)
        return L"amd_fidelityfx_dx12d.dll";
#else
        return L"amd_fidelityfx_dx12.dll";
#endif
    default:
        FSR_ERROR("Unsupported fsrapi backend");
        return L"";
    }
}

ffxReturnCode_t ffxCreateContext(ffxContext* context, ffxCreateContextDescHeader* desc, const ffxAllocationCallbacks* memCb)
{
    static PfnFfxCreateContext createContext = reinterpret_cast<PfnFfxCreateContext>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxCreateContext"));
    return createContext(context, desc, memCb);
}

ffxReturnCode_t ffxDestroyContext(ffxContext* context, const ffxAllocationCallbacks* memCb)
{
    static PfnFfxDestroyContext destroyContext = reinterpret_cast<PfnFfxDestroyContext>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxDestroyContext"));
    return destroyContext(context, memCb);
}

ffxReturnCode_t ffxConfigure(ffxContext* context, const ffxConfigureDescHeader* desc)
{
    static PfnFfxConfigure configure = reinterpret_cast<PfnFfxConfigure>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxConfigure"));
    return configure(context, desc);
}

ffxReturnCode_t ffxQuery(ffxContext* context, ffxQueryDescHeader* desc)
{
    static PfnFfxQuery query = reinterpret_cast<PfnFfxQuery>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxQuery"));
    return query(context, desc);
}

ffxReturnCode_t ffxDispatch(ffxContext* context, const ffxDispatchDescHeader* desc)
{
    static PfnFfxDispatch dispatch = reinterpret_cast<PfnFfxDispatch>(DllLoader::Instance(GetDllName().c_str()).GetProcAddress("ffxDispatch"));
    return dispatch(context, desc);
}
#endif
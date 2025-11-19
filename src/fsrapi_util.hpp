#pragma once

#if defined(FSR_BACKEND_DX12) || defined(FSR_BACKEND_ALL)
#include <d3d12.h>

#define FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12 0x0000002u
struct ffxCreateBackendDX12Desc
{
    ffxCreateContextDescHeader header;
    ID3D12Device* device;  ///< Device on which the backend will run.
};

namespace ffx
{
    template<>
    struct struct_type<ffxCreateBackendDX12Desc> : std::integral_constant<uint64_t, FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_DX12> {};
    struct CreateBackendDX12Desc : public InitHelper<ffxCreateBackendDX12Desc> {};
}

static inline uint32_t ffxApiGetSurfaceFormatDX12(DXGI_FORMAT format)
{
    switch (format) {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R32G32B32A32_TYPELESS;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return FFX_API_SURFACE_FORMAT_R32G32B32A32_FLOAT;
    case DXGI_FORMAT_R32G32B32A32_UINT:
        return FFX_API_SURFACE_FORMAT_R32G32B32A32_UINT;
        //case DXGI_FORMAT_R32G32B32A32_SINT:
        //case DXGI_FORMAT_R32G32B32_TYPELESS:
        //case DXGI_FORMAT_R32G32B32_FLOAT:
        //case DXGI_FORMAT_R32G32B32_UINT:
        //case DXGI_FORMAT_R32G32B32_SINT:

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R16G16B16A16_TYPELESS;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT;
        //case DXGI_FORMAT_R16G16B16A16_UNORM:
        //case DXGI_FORMAT_R16G16B16A16_UINT:
        //case DXGI_FORMAT_R16G16B16A16_SNORM:
        //case DXGI_FORMAT_R16G16B16A16_SINT:

    case DXGI_FORMAT_R32G32_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R32G32_TYPELESS;
    case DXGI_FORMAT_R32G32_FLOAT:
        return FFX_API_SURFACE_FORMAT_R32G32_FLOAT;
        //case DXGI_FORMAT_R32G32_FLOAT:
        //case DXGI_FORMAT_R32G32_UINT:
        //case DXGI_FORMAT_R32G32_SINT:

    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R32_FLOAT;

    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R32_UINT;

    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        return FFX_API_SURFACE_FORMAT_R8_UINT;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R10G10B10A2_TYPELESS;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return FFX_API_SURFACE_FORMAT_R10G10B10A2_UNORM;
        //case DXGI_FORMAT_R10G10B10A2_UINT:

    case DXGI_FORMAT_R11G11B10_FLOAT:
        return FFX_API_SURFACE_FORMAT_R11G11B10_FLOAT;

    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R8G8B8A8_TYPELESS;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return FFX_API_SURFACE_FORMAT_R8G8B8A8_SRGB;
        //case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return FFX_API_SURFACE_FORMAT_R8G8B8A8_SNORM;

    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        return FFX_API_SURFACE_FORMAT_B8G8R8A8_TYPELESS;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        return FFX_API_SURFACE_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return FFX_API_SURFACE_FORMAT_B8G8R8A8_SRGB;

    case DXGI_FORMAT_R16G16_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R16G16_TYPELESS;
    case DXGI_FORMAT_R16G16_FLOAT:
        return FFX_API_SURFACE_FORMAT_R16G16_FLOAT;
        //case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
        return FFX_API_SURFACE_FORMAT_R16G16_UINT;
        //case DXGI_FORMAT_R16G16_SNORM
        //case DXGI_FORMAT_R16G16_SINT 

        //case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R32_UINT:
        return FFX_API_SURFACE_FORMAT_R32_UINT;
    case DXGI_FORMAT_R32_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R32_TYPELESS;
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
        return FFX_API_SURFACE_FORMAT_R32_FLOAT;

    case DXGI_FORMAT_R8G8_UINT:
        return FFX_API_SURFACE_FORMAT_R8G8_UINT;
    case DXGI_FORMAT_R8G8_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R8G8_TYPELESS;
    case DXGI_FORMAT_R8G8_UNORM:
        return FFX_API_SURFACE_FORMAT_R8G8_UNORM;
        //case DXGI_FORMAT_R8G8_SNORM:
        //case DXGI_FORMAT_R8G8_SINT:

    case DXGI_FORMAT_R16_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R16_TYPELESS;
    case DXGI_FORMAT_R16_FLOAT:
        return FFX_API_SURFACE_FORMAT_R16_FLOAT;
    case DXGI_FORMAT_R16_UINT:
        return FFX_API_SURFACE_FORMAT_R16_UINT;
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
        return FFX_API_SURFACE_FORMAT_R16_UNORM;
    case DXGI_FORMAT_R16_SNORM:
        return FFX_API_SURFACE_FORMAT_R16_SNORM;
        //case DXGI_FORMAT_R16_SINT:

    case DXGI_FORMAT_R8_TYPELESS:
        return FFX_API_SURFACE_FORMAT_R8_TYPELESS;
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_A8_UNORM:
        return FFX_API_SURFACE_FORMAT_R8_UNORM;
    case DXGI_FORMAT_R8_UINT:
        return FFX_API_SURFACE_FORMAT_R8_UINT;
        //case DXGI_FORMAT_R8_SNORM:
        //case DXGI_FORMAT_R8_SINT:
        //case DXGI_FORMAT_R1_UNORM:

    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        return FFX_API_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP;

    case DXGI_FORMAT_UNKNOWN:
    default:
        return FFX_API_SURFACE_FORMAT_UNKNOWN;
    }
}

static inline FfxApiResource ffxApiGetResourceDX12(ID3D12Resource* pRes, uint32_t state = FFX_API_RESOURCE_STATE_COMPUTE_READ, uint32_t additionalUsages = 0)
{
    FfxApiResource res{};
    res.resource = pRes;
    res.state = state;
    if (!pRes) return res;

    D3D12_RESOURCE_DESC desc = pRes->GetDesc();
    if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
        res.description.flags = FFX_API_RESOURCE_FLAGS_NONE;
        res.description.usage = FFX_API_RESOURCE_USAGE_UAV;
        res.description.size = static_cast<uint32_t>(desc.Width);
        res.description.stride = static_cast<uint32_t>(desc.Height);
        res.description.type = FFX_API_RESOURCE_TYPE_BUFFER;
    } else {
        res.description.flags = FFX_API_RESOURCE_FLAGS_NONE;
        if (desc.Format == DXGI_FORMAT_D16_UNORM || desc.Format == DXGI_FORMAT_D32_FLOAT) {
            res.description.usage = FFX_API_RESOURCE_USAGE_DEPTHTARGET;
        } else if (desc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || desc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT) {
            res.description.usage = FFX_API_RESOURCE_USAGE_DEPTHTARGET | FFX_API_RESOURCE_USAGE_STENCILTARGET;
        } else {
            res.description.usage = FFX_API_RESOURCE_USAGE_READ_ONLY;
        }

        if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
            res.description.usage |= FFX_API_RESOURCE_USAGE_UAV;

        res.description.width = static_cast<uint32_t>(desc.Width);
        res.description.height = static_cast<uint32_t>(desc.Height);
        res.description.depth = static_cast<uint32_t>(desc.DepthOrArraySize);
        res.description.mipCount = static_cast<uint32_t>(desc.MipLevels);

        switch (desc.Dimension) {
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            res.description.type = FFX_API_RESOURCE_TYPE_TEXTURE1D;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            if (desc.DepthOrArraySize == 6)
                res.description.type = FFX_API_RESOURCE_TYPE_TEXTURE_CUBE;
            else
                res.description.type = FFX_API_RESOURCE_TYPE_TEXTURE2D;
            break;
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            res.description.type = FFX_API_RESOURCE_TYPE_TEXTURE3D;
            break;
        }
    }

    res.description.format = ffxApiGetSurfaceFormatDX12(desc.Format);
    res.description.usage |= additionalUsages;
    return res;
}
#endif

#if defined(FSR_BACKEND_VK) || defined(FSR_BACKEND_ALL)
#include <vulkan/vulkan.h>

#define FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK 0x0000003u
struct ffxCreateBackendVKDesc
{
    ffxCreateContextDescHeader header;
    VkDevice                   vkDevice;          ///< the logical device used by the program.
    VkPhysicalDevice           vkPhysicalDevice;  ///< the physical device used by the program.
    PFN_vkGetDeviceProcAddr    vkDeviceProcAddr;  ///< function pointer to get device procedure addresses
};

namespace ffx
{
    template<>
    struct struct_type<ffxCreateBackendVKDesc> : std::integral_constant<uint64_t, FFX_API_CREATE_CONTEXT_DESC_TYPE_BACKEND_VK> {};
    struct CreateBackendVKDesc : public InitHelper<ffxCreateBackendVKDesc> {};
}

static inline uint32_t ffxApiGetSurfaceFormatVK(VkFormat fmt)
{
    switch (fmt) {
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return FFX_API_SURFACE_FORMAT_R32G32B32A32_FLOAT;
    case VK_FORMAT_R32G32B32_SFLOAT:
        return FFX_API_SURFACE_FORMAT_R32G32B32_FLOAT;
    case VK_FORMAT_R32G32B32A32_UINT:
        return FFX_API_SURFACE_FORMAT_R32G32B32A32_UINT;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return FFX_API_SURFACE_FORMAT_R16G16B16A16_FLOAT;
    case VK_FORMAT_R32G32_SFLOAT:
        return FFX_API_SURFACE_FORMAT_R32G32_FLOAT;
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
        return FFX_API_SURFACE_FORMAT_R32_UINT;
    case VK_FORMAT_R8G8B8A8_UNORM:
        return FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM;
    case VK_FORMAT_R8G8B8A8_SNORM:
        return FFX_API_SURFACE_FORMAT_R8G8B8A8_SNORM;
    case VK_FORMAT_R8G8B8A8_SRGB:
        return FFX_API_SURFACE_FORMAT_R8G8B8A8_SRGB;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return FFX_API_SURFACE_FORMAT_B8G8R8A8_UNORM;
    case VK_FORMAT_B8G8R8A8_SRGB:
        return FFX_API_SURFACE_FORMAT_B8G8R8A8_SRGB;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return FFX_API_SURFACE_FORMAT_R11G11B10_FLOAT;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        return FFX_API_SURFACE_FORMAT_R10G10B10A2_UNORM;
    case VK_FORMAT_R16G16_SFLOAT:
        return FFX_API_SURFACE_FORMAT_R16G16_FLOAT;
    case VK_FORMAT_R16G16_UINT:
        return FFX_API_SURFACE_FORMAT_R16G16_UINT;
    case VK_FORMAT_R16G16_SINT:
        return FFX_API_SURFACE_FORMAT_R16G16_SINT;
    case VK_FORMAT_R16_SFLOAT:
        return FFX_API_SURFACE_FORMAT_R16_FLOAT;
    case VK_FORMAT_R16_UINT:
        return FFX_API_SURFACE_FORMAT_R16_UINT;
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D16_UNORM_S8_UINT:
        return FFX_API_SURFACE_FORMAT_R16_UNORM;
    case VK_FORMAT_R16_SNORM:
        return FFX_API_SURFACE_FORMAT_R16_SNORM;
    case VK_FORMAT_R8_UNORM:
        return FFX_API_SURFACE_FORMAT_R8_UNORM;
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_S8_UINT:
        return FFX_API_SURFACE_FORMAT_R8_UINT;
    case VK_FORMAT_R8G8_UNORM:
        return FFX_API_SURFACE_FORMAT_R8G8_UNORM;
    case VK_FORMAT_R8G8_UINT:
        return FFX_API_SURFACE_FORMAT_R8G8_UINT;
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return FFX_API_SURFACE_FORMAT_R32_FLOAT;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        return FFX_API_SURFACE_FORMAT_R9G9B9E5_SHAREDEXP;
    case VK_FORMAT_UNDEFINED:
        return FFX_API_SURFACE_FORMAT_UNKNOWN;

    default:
        // NOTE: we do not support typeless formats here
        FSR_ERROR("Format not yet supported");
        return FFX_API_SURFACE_FORMAT_UNKNOWN;
    }
}

static inline uint32_t ffxApiGetSurfaceFormatToGamma(uint32_t fmt)
{
    switch (fmt) {
    case (FFX_API_SURFACE_FORMAT_R8G8B8A8_UNORM):
        return FFX_API_SURFACE_FORMAT_R8G8B8A8_SRGB;
    case (FFX_API_SURFACE_FORMAT_B8G8R8A8_UNORM):
        return FFX_API_SURFACE_FORMAT_B8G8R8A8_SRGB;
    default:
        return fmt;
    }
}

static inline bool ffxApiIsDepthFormat(VkFormat fmt)
{
    switch (fmt) {
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;

    default:
        return false;
    }
}

static inline bool ffxApiIsStencilFormat(VkFormat fmt)
{
    switch (fmt) {
    case VK_FORMAT_S8_UINT:
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return true;

    default:
        return false;
    }
}

static inline FfxApiResourceDescription ffxApiGetImageResourceDescriptionVK(const VkImage image, const VkImageCreateInfo createInfo, uint32_t additionalUsages)
{
    FfxApiResourceDescription resourceDescription = {};

    // This is valid
    if (image == VK_NULL_HANDLE)
        return resourceDescription;

    // Set flags properly for resource registration
    resourceDescription.flags = FFX_API_RESOURCE_FLAGS_NONE;
    resourceDescription.usage = FFX_API_RESOURCE_USAGE_READ_ONLY;

    // Check for depth stencil use
    if (ffxApiIsDepthFormat(createInfo.format))
        resourceDescription.usage |= FFX_API_RESOURCE_USAGE_DEPTHTARGET;
    if (ffxApiIsStencilFormat(createInfo.format))
        resourceDescription.usage |= FFX_API_RESOURCE_USAGE_STENCILTARGET;

    // Unordered access use
    if ((createInfo.usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0)
        resourceDescription.usage |= FFX_API_RESOURCE_USAGE_UAV;

    // Resource-specific supplemental use flags
    resourceDescription.usage |= additionalUsages;

    resourceDescription.width = createInfo.extent.width;
    resourceDescription.height = createInfo.extent.height;
    resourceDescription.mipCount = createInfo.mipLevels;
    resourceDescription.format = ffxApiGetSurfaceFormatVK(createInfo.format);

    // if the mutable flag is present, assume that the real format is sRGB
    if ((createInfo.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) != 0)
        resourceDescription.format = ffxApiGetSurfaceFormatToGamma(resourceDescription.format);

    switch (createInfo.imageType) {
    case VK_IMAGE_TYPE_1D:
        resourceDescription.type = FFX_API_RESOURCE_TYPE_TEXTURE1D;
        break;
    case VK_IMAGE_TYPE_2D:
        resourceDescription.depth = createInfo.arrayLayers;
        if ((additionalUsages & FFX_API_RESOURCE_USAGE_ARRAYVIEW) != 0)
            resourceDescription.type = FFX_API_RESOURCE_TYPE_TEXTURE2D;
        else if ((createInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0)
            resourceDescription.type = FFX_API_RESOURCE_TYPE_TEXTURE_CUBE;
        else
            resourceDescription.type = FFX_API_RESOURCE_TYPE_TEXTURE2D;
        break;
    case VK_IMAGE_TYPE_3D:
        resourceDescription.depth = createInfo.extent.depth;
        resourceDescription.type = FFX_API_RESOURCE_TYPE_TEXTURE3D;
        break;
    default:
        FSR_ERROR("FFXInterface: VK: Unsupported texture dimension requested. Please implement.");
        break;
    }

    return resourceDescription;
}

static inline FfxApiResource ffxApiGetResourceVK(void* vkResource, FfxApiResourceDescription ffxResDescription, uint32_t state)
{
    FfxApiResource resource = {};
    resource.resource = vkResource;
    resource.state = state;
    resource.description = ffxResDescription;

    return resource;
}
#endif
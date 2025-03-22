#pragma once

#include <array>
#include <vector>

#include "IUnityInterface.h"
#include "ffx_upscale.hpp"


enum TextureName
{
    INVALID = 0,
    COLOR = 1,
    DEPTH,
    MOTION_VECTORS,
    REACTIVE,
    TRANSPARENT_AND_COMPOSITION,
    OUTPUT,
    COLOR_OPAQUE_ONLY,
    COLOR_PRE_UPSCALE,
    MAX
};

struct InitParam
{
    uint32_t flags;
    uint32_t displaySizeWidth;
    uint32_t displaySizeHeight;
};

struct GenReactiveParam
{
    void* colorOpaqueOnly;
    void* colorPreUpscale;
    void* outReactive;
    uint32_t renderSizeWidth;
    uint32_t renderSizeHeight;
    float scale;
    float cutoffThreshold;
    float binaryValue;
    uint32_t flags;
};

struct DispatchParam
{
    void* color;
    void* depth;
    void* motionVectors;
    void* reactive;
    void* transparencyAndComposition;
    void* output;
    float jitterOffsetX;
    float jitterOffsetY;
    float motionVectorScaleX;
    float motionVectorScaleY;
    uint32_t renderSizeWidth;
    uint32_t renderSizeHeight;
    bool enableSharpening;
    float sharpness;
    float frameTimeDelta;
    float cameraNear;
    float cameraFar;
    float cameraFovAngleVertical;
};

class FSRAPI
{
public:
    ~FSRAPI() { Destroy(); }
    ffx::ReturnCode Init(const InitParam& initParam, uint32_t fsrVersion = 0);
    void Destroy();
    std::array<float, 2> GetJitterOffset(const int32_t index, const int32_t renderWidth, const int32_t displayWidth);
    ffx::ReturnCode GenerateReactiveMask(const GenReactiveParam& genReactiveParam);
    ffx::ReturnCode Dispatch(const DispatchParam& dispatchParam);
    void SetTextureID(const TextureName textureName, const UnityTextureID textureID);

private:
    ffx::Context m_Context;
    bool m_ContextCreated = false;
    bool m_Reset = true;

    std::array<uint32_t, TextureName::MAX> m_TextureIDs = {};
};

FSRAPI& GetFSRInstance(uint32_t id);

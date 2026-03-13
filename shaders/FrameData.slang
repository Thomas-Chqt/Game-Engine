/*
 * ---------------------------------------------------
 * FrameData.slang
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef FRAMEDATA_SLANG
#define FRAMEDATA_SLANG

#include "shader.h"

SLANG_MODULE_DEF(frameData);
SLANG_MODULE_IMP(light);

namespace shader
{

SLANG_PUBLIC struct FrameDataConstantBuffer
{
    SLANG_PUBLIC float4x4 vpMatrix;
    SLANG_PUBLIC float3 cameraPosition;    FLOAT3_PADDING(0);
    SLANG_PUBLIC float3 ambientLightColor; FLOAT3_PADDING(1);
    SLANG_PUBLIC int directionalLightCount;
    SLANG_PUBLIC int pointLightCount;
};

#ifndef __cplusplus
SLANG_PUBLIC struct FrameData
{
    SLANG_PUBLIC cbuffer constant
    {
        SLANG_PUBLIC float4x4 vpMatrix;
        SLANG_PUBLIC float3 cameraPosition;    FLOAT3_PADDING(0);
        SLANG_PUBLIC float3 ambientLightColor; FLOAT3_PADDING(1);
        SLANG_PUBLIC int directionalLightCount;
        SLANG_PUBLIC int pointLightCount;
    };
    SLANG_PUBLIC StructuredBuffer<DirectionalLight> directionalLights;
    SLANG_PUBLIC StructuredBuffer<PointLight> pointLights;
};
#endif

}
#endif

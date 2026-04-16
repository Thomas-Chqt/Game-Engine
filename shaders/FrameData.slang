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

SLANG_MODULE_DEF(FrameData);
SLANG_MODULE_IMP(Light);

namespace shader
{

SLANG_PUBLIC struct FrameData
{
    SLANG_PUBLIC CBUFFER_BEGIN(_)
        SLANG_PUBLIC float4x4 vpMatrix;
        SLANG_PUBLIC float3 cameraPosition;    FLOAT3_PADDING(0);
        SLANG_PUBLIC float3 ambientLightColor; FLOAT3_PADDING(1);
        SLANG_PUBLIC int directionalLightCount;
        SLANG_PUBLIC int pointLightCount;
    CBUFFER_END
    #ifndef __cplusplus
        SLANG_PUBLIC  StructuredBuffer<DirectionalLight> directionalLights;
        SLANG_PUBLIC  StructuredBuffer<PointLight> pointLights;
    #endif
};

}
#endif

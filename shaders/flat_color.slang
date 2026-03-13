/*
 * ---------------------------------------------------
 * flat_color.slang
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "shader.h"

import frameData;
import material;
import phong_shading;

namespace shader
{

ParameterBlock<FrameData> frameData;
ParameterBlock<StructuredBuffer<float4x4>> modelMatrices;
ParameterBlock<StructuredBuffer<FlatColorMaterial>> materials;

PUSH_CONSTANT
{
    uint modelMatrixIndex;
    uint materialIndex;
}

/* -------------------------Vertex Shader------------------------------- */

struct Vertex
{
    float3 pos;
    float3 normal;
};

struct VSOutput
{
    float3 pos;
    float4 clipPos : SV_Position;
    float3 normal;
};

[shader("vertex")]
VSOutput vertexMain(Vertex input)
{
    float4 worldPos  = mul(float4(input.pos, 1.0), modelMatrices[modelMatrixIndex]);

    VSOutput output;
    output.pos     = worldPos.xyz;
    output.clipPos = mul(worldPos, frameData.vpMatrix);
    output.normal  = mul(input.normal, (float3x3)modelMatrices[modelMatrixIndex]);
    return output;
}

/* -------------------------Fragment Shader------------------------------- */

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_TARGET
{
    if (materials[materialIndex].diffuseColor.a < 1.0f)
        discard;

    phong::FragmentContext fragCtx;
    fragCtx.position      = input.pos;
    fragCtx.normal        = normalize(input.normal);
    fragCtx.cameraDir     = normalize(frameData.cameraPosition - input.pos);
    fragCtx.diffuseColor  = materials[materialIndex].diffuseColor.rgb;
    fragCtx.specularColor = materials[materialIndex].specularColor;
    fragCtx.shininess     = materials[materialIndex].shininess;

    float3 finalColor = frameData.ambientLightColor * fragCtx.diffuseColor;
    for (int i = 0; i < frameData.directionalLightCount; ++i)
        finalColor += phong::evalDirectionalLight(fragCtx, frameData.directionalLights[i]);
    for (int i = 0; i < frameData.pointLightCount; ++i)
        finalColor += phong::evalPointLight(fragCtx, frameData.pointLights[i]);

    return float4(saturate(finalColor), materials[materialIndex].diffuseColor.a);
}

}

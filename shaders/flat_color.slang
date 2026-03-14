/*
 * ---------------------------------------------------
 * flat_color.slang
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "shader.h"

SLANG_MODULE_IMP(light);
SLANG_MODULE_IMP(frameData);
SLANG_MODULE_IMP(phong_shading);

namespace shader::flat_color
{

/* -------------------------Vertex Shader------------------------------- */

#ifndef __cplusplus

ParameterBlock<FrameData> frameData;

PUSH_CONSTANT
{
    float4x4 modelMatrix;
}

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
    float4 worldPos  = mul(float4(input.pos, 1.0), modelMatrix);

    VSOutput output;
    output.pos     = worldPos.xyz;
    output.clipPos = mul(worldPos, frameData.vpMatrix);
    output.normal  = mul(input.normal, (float3x3)modelMatrix);
    return output;
}

#endif // ifndef _cplusplus

/* -------------------------Fragment Shader------------------------------- */

struct Material
{
    float4 diffuseColor;
    float3 specularColor; FLOAT3_PADDING(0);
    float shininess;
};

#ifndef __cplusplus

ParameterBlock<ConstantBuffer<Material>> material;

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_TARGET
{
    if (material.diffuseColor.a < 1.0f)
        discard;

    phong::FragmentContext fragCtx;
    fragCtx.position      = input.pos;
    fragCtx.normal        = normalize(input.normal);
    fragCtx.cameraDir     = normalize(frameData.cameraPosition - input.pos);
    fragCtx.diffuseColor  = material.diffuseColor.rgb;
    fragCtx.specularColor = material.specularColor;
    fragCtx.shininess     = material.shininess;

    float3 finalColor = frameData.ambientLightColor * fragCtx.diffuseColor;
    for (int i = 0; i < frameData.directionalLightCount; ++i)
        finalColor += phong::evalDirectionalLight(fragCtx, frameData.directionalLights[i]);
    for (int i = 0; i < frameData.pointLightCount; ++i)
        finalColor += phong::evalPointLight(fragCtx, frameData.pointLights[i]);

    return float4(saturate(finalColor), material.diffuseColor.a);
}

#endif // ifndef _cplusplus

} // namespace shader::flat_color

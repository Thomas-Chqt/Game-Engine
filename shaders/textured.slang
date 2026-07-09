#include "shader.h"

SLANG_MODULE_IMP(Light);
SLANG_MODULE_IMP(FrameData);
SLANG_MODULE_IMP(phong_shading);

namespace shader::textured
{

struct alignas(16) Material
{
    float4 diffuseColor;
    uint32_t diffuseTextureIdx;  UINT32_PADDING(0);
    float3 specularColor;        FLOAT3_PADDING(0);
    float3 emissiveColor;        FLOAT3_PADDING(1);
    uint32_t emissiveTextureIdx;
    float shininess;
};

/* -------------------------Vertex Shader------------------------------- */

#ifndef __cplusplus

struct TextureArray
{
    SamplerState sampler;
    Texture2D<float4> textures[4096];
};

ParameterBlock<TextureArray> textureArray;
ParameterBlock<FrameData> frameData;
ParameterBlock<StructuredBuffer<Material>> materials;

PUSH_CONSTANT
{
    uint32_t materialIdx;
    uint2 entityId;
    float4x4 modelMatrix;
}

struct Vertex
{
    float3 pos;
    float2 uv;
    float3 normal;
};

struct VSOutput
{
    float3 pos;
    float4 clipPos : SV_Position;
    float2 uv;
    float3 normal;
};

[shader("vertex")]
VSOutput vertexMain(Vertex input)
{
    float4 worldPos  = mul(float4(input.pos, 1.0), modelMatrix);

    VSOutput output;
    output.pos     = worldPos.xyz;
    output.clipPos = mul(worldPos, frameData.vpMatrix);
    output.uv      = input.uv;
    output.normal  = mul(input.normal, (float3x3)modelMatrix);
    return output;
}

/* -------------------------Fragment Shader------------------------------- */

struct FSOutput
{
    float4 color : SV_Target0;
    uint2 entityId : SV_Target1;
};

[shader("fragment")]
FSOutput fragmentMain(VSOutput input)
{
    let sampler: SamplerState = textureArray.sampler;

    let diffuseColor: float4 = materials[materialIdx].diffuseColor;
    let diffuseTexture: Texture2D<float4> = textureArray.textures[materials[materialIdx].diffuseTextureIdx];

    let specularColor: float3 = materials[materialIdx].specularColor;

    let emissiveColor: float3 = materials[materialIdx].emissiveColor;
    let emissiveTexture: Texture2D<float4> = textureArray.textures[materials[materialIdx].emissiveTextureIdx];

    let shininess: float = materials[materialIdx].shininess;


    float4 texColor = diffuseColor * diffuseTexture.Sample(sampler, input.uv);
    if (texColor.a < 0.5)
        discard;

    phong::FragmentContext fragCtx;
    fragCtx.position      = input.pos;
    fragCtx.normal        = normalize(input.normal);
    fragCtx.cameraDir     = normalize(frameData.cameraPosition - input.pos);
    fragCtx.diffuseColor  = texColor.rgb;
    fragCtx.specularColor = specularColor;
    fragCtx.shininess     = shininess;

    float3 finalColor = frameData.ambientLightColor * fragCtx.diffuseColor;
    for (int i = 0; i < frameData.directionalLightCount; ++i)
        finalColor += phong::evalDirectionalLight(fragCtx, frameData.directionalLights[i]);
    for (int i = 0; i < frameData.pointLightCount; ++i)
        finalColor += phong::evalPointLight(fragCtx, frameData.pointLights[i]);
    finalColor += emissiveColor * emissiveTexture.Sample(sampler, input.uv).rgb;

    FSOutput output;
    output.color = float4(saturate(finalColor), diffuseColor.a);
    output.entityId = entityId;
    return output;
}

#endif // ifndef _cplusplus

} // namespace shader::textured

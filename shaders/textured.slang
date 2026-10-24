#include "shader.h"

SLANG_MODULE_IMP(Light);
SLANG_MODULE_IMP(FrameData);
SLANG_MODULE_IMP(pbr);
SLANG_MODULE_IMP(ibl);
SLANG_MODULE_IMP(utils);

namespace shader::textured
{

struct alignas(16) Material
{
    float4 baseColor;
    float3 emissiveColor;        FLOAT3_PADDING(0);
    float metallic;
    float roughness;
    float normalScale;
    uint32_t baseColorTextureIdx;
    uint32_t emissiveTextureIdx;
    uint32_t metallicRoughnessTextureIdx;
    uint32_t normalTextureIdx;
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

struct Environment
{
    TextureCube<float4> irradianceCubeMap;
    TextureCube<float4> prefilteredEnvironmentCubeMap;
    Texture2D<float2> brdfLut;
    SamplerState sampler;
};
ParameterBlock<Environment> environment;

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
    float4 tangent;
};

struct VSOutput
{
    float3 pos;
    float4 clipPos : SV_Position;
    float2 uv;
    float3 normal;
    float3 tangent;
    float3 bitangent;
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
    output.tangent = mul(input.tangent.xyz, (float3x3)modelMatrix);
    output.bitangent = mul(cross(input.normal, input.tangent.xyz) * input.tangent.w, (float3x3)modelMatrix);
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
    const Material material = materials[materialIdx];
    const float4 baseColor = material.baseColor * textureArray.textures[material.baseColorTextureIdx].Sample(textureArray.sampler, input.uv);
    if (baseColor.a < 0.5)
        discard;

    const float3 emissive = material.emissiveColor * textureArray.textures[material.emissiveTextureIdx].Sample(textureArray.sampler, input.uv).rgb;
    const float4 metallicRoughness = textureArray.textures[material.metallicRoughnessTextureIdx].Sample(textureArray.sampler, input.uv);
    const float metallic = saturate(material.metallic * metallicRoughness.b);
    const float roughness = max(material.roughness * metallicRoughness.g, 0.01);

    float3 N = normalize(input.normal);
    if (dot(input.tangent, input.tangent) > 0.0 && dot(input.bitangent, input.bitangent) > 0.0)
    {
        float3 normalMap = textureArray.textures[material.normalTextureIdx].Sample(textureArray.sampler, input.uv).rgb * 2.0 - 1.0;
        normalMap.xy *= material.normalScale;
        const float3 T = normalize(input.tangent);
        const float3 B = normalize(input.bitangent);
        N = normalize(mul(normalMap, float3x3(T, B, N)));
    }
    const float3 V = normalize(frameData.cameraPosition - input.pos);
    const float NdotV = max(dot(N, V), 0.0);

    pbr::FragmentContext fragCtx;
    fragCtx.position = input.pos;
    fragCtx.normal = N;
    fragCtx.cameraDir = V;
    fragCtx.baseColor = baseColor.rgb;
    fragCtx.metallic = metallic;
    fragCtx.roughness = roughness;

    float3 Lo = float3(0.0);
    for (int i = 0; i < frameData.directionalLightCount; ++i)
        Lo += pbr::evalDirectionalLight(fragCtx, frameData.directionalLights[i]);
    for (int i = 0; i < frameData.pointLightCount; ++i)
        Lo += pbr::evalPointLight(fragCtx, frameData.pointLights[i]);

    uint prefilteredWidth;
    uint prefilteredHeight;
    uint prefilteredMipCount;
    environment.prefilteredEnvironmentCubeMap.GetDimensions(0, prefilteredWidth, prefilteredHeight, prefilteredMipCount);

    const float3 irradiance = environment.irradianceCubeMap.Sample(environment.sampler, N).rgb;
    const float3 R = reflect(-V, N);
    const float mipLevel = roughness * float(prefilteredMipCount - 1);
    const float3 prefilteredColor = environment.prefilteredEnvironmentCubeMap.SampleLevel(environment.sampler, R, mipLevel).rgb;
    const float2 environmentBrdf = environment.brdfLut.SampleLevel(environment.sampler, float2(NdotV, roughness), 0.0).rg;
    const float3 F0 = lerp(float3(0.04), baseColor.rgb, metallic);
    const float3 ambient = ibl::evaluateIBL(
        baseColor.rgb, metallic, roughness, NdotV, F0,
        irradiance, prefilteredColor, environmentBrdf
    );

    FSOutput output;
    output.color = float4(acesToneMapping((Lo + ambient + emissive) * 0.5), baseColor.a);
    output.entityId = entityId;
    return output;
}

#endif // ifndef _cplusplus

} // namespace shader::textured

/*
 * ---------------------------------------------------
 * default.metal
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/07/07 09:27:04
 * ---------------------------------------------------
 */

#include <metal_stdlib>

using namespace metal;

struct VertexIn
{
    float3 pos     [[attribute(0)]];
    float2 uv      [[attribute(1)]];
    float3 normal  [[attribute(2)]];
};

struct VertexOut
{
    float4   clipPos [[position]];
    float3   pos;
    float2   uv;
    float3   normal;
};

vertex VertexOut default_vs(VertexIn in [[stage_in]],
    constant float4x4& vpMatrixBuffer    [[buffer(1)]],
    constant float4x4& modelMatrix [[buffer(2)]]
)
{
    float4 worldPos  = modelMatrix * float4(in.pos, 1.0);

    return (VertexOut){
        .clipPos   = vpMatrixBuffer * worldPos,
        .pos       = worldPos.xyz,
        .uv        = in.uv,
        .normal    = (modelMatrix * float4(in.normal,  0)).xyz,
    };
}

struct LightsBuffer
{
    struct {
        float3 pos;
        float3 color;
        float intensity;
    }
    pointLights[32];
    uint pointLightCount;
};

fragment float4 default_fs(VertexOut in [[stage_in]],
    constant LightsBuffer& lightsBuffer [[buffer(1)]]
)
{
    float3 fragNormal = normalize(in.normal);
    float3 fragDiffuse = float3(1.0f, 1.0f, 1.0f);

    float3 finalColor = float3(0.0f, 0.0f, 0.0f);

    for (uint i = 0; i < lightsBuffer.pointLightCount; i++)
    {
        constant auto& light = lightsBuffer.pointLights[i];

        float diffuseFactor = dot(fragNormal, normalize(light.pos - in.pos));

        float3 ambient = fragDiffuse * light.color * (light.intensity * 0.2);
        float3 diffuse = fragDiffuse * light.color * light.intensity * max(diffuseFactor, 0.0f);
        
        finalColor += ambient + diffuse;
    }

    return float4(finalColor, 1.0F);
}
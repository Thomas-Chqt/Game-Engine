/*
 * ---------------------------------------------------
 * default.fs
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/07/13 11:42:39
 * ---------------------------------------------------
 */

#version 410 core

in VertexOut
{
    vec3 pos;
    vec2 uv;
    vec3 tangent;
    vec3 bitangent;
    vec3 normal;
} fsIn;

out vec4 finalColor;

struct PointLight
{
    vec3  pos;
    float _pad0;
    vec3  color;
    float _pad1;
    float intentsity;
};

layout (std140) uniform lightsBuffer
{
    PointLight pointLights[32];
    uint pointLightCount;
};


void main()
{
    vec3 fragNormal = normalize(fsIn.normal);
    vec3 fragDiffuse = vec3(1.0f, 1.0f, 1.0f);

    finalColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    for(uint i = 0; i < pointLightCount; i++)
    {
        PointLight light = pointLights[i];

        float diffuseFactor = dot(fragNormal, normalize(light.pos - fsIn.pos));

        vec3 ambiant = fragDiffuse * light.color * (light.intentsity * 0.2);
        vec3 diffuse = fragDiffuse * light.color * light.intentsity * max(diffuseFactor, 0.0f);

        finalColor += vec4(ambiant + diffuse, 1.0F);
    }
}
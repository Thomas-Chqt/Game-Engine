#pragma once

#include "Game-Engine/AssetManager.hpp"
#include <glm/glm.hpp>

namespace GE
{

struct Material
{
    glm::vec4 baseColor;
    AssetID baseColorTexture;

    float metallic;
    float roughness;
    AssetID metallicRoughnessTexture;
    AssetID normalTexture;
    float normalScale;

    glm::vec3 emissiveColor;
    AssetID emissiveTexture;

};

}

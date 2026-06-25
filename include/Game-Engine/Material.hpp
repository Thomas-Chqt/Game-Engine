#pragma once

#include "Game-Engine/AssetManager.hpp"
#include <glm/glm.hpp>

namespace GE
{

struct Material
{
    glm::vec4 diffuseColor;
    AssetID diffuseTexture;

    glm::vec3 specularColor;

    glm::vec3 emissiveColor;

    float shininess;
};

}

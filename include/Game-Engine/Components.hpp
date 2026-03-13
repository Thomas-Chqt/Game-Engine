/*
 * ---------------------------------------------------
 * Components.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/18 17:35:21
 * ---------------------------------------------------
 */

#ifndef COMPONENTS_HPP
#define COMPONENTS_HPP

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/ECSWorld.hpp"

#include <glm/glm.hpp>

#include <numbers>
#include <string>

namespace GE
{

struct NameComponent
{
    std::string name;
};

struct HierarchyComponent
{
    ECSWorld::EntityID parent = INVALID_ENTITY_ID;
    ECSWorld::EntityID firstChild = INVALID_ENTITY_ID;
    ECSWorld::EntityID nextChild = INVALID_ENTITY_ID;
};

struct TransformComponent
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};
};

struct CameraComponent
{
    float fov   = (float)(60 * (std::numbers::pi_v<float> / 180.0f));
    float zFar  = 1000.0f;
    float zNear = 0.1f;
};

struct LightComponent
{
    enum class Type : glm::uint8_t { point = 0 } type = LightComponent::Type::point;

    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intentsity = 1.0f;
};

struct MeshComponent
{
    AssetID mesh;
};

} // namespace GE

#endif // COMPONENTS_HPP

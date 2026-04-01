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

#include "Game-Engine/AssetManagerView.hpp"
#include "Game-Engine/ECSWorld.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <variant>

namespace GE
{

struct NameComponent
{
    std::string name;

    inline operator std::string& () { return name; }
    inline operator const std::string& () const { return name; }
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

    inline operator glm::mat4 () const
    {
        auto matrix = glm::mat4x4(1.0f);
        matrix = glm::translate(matrix, position);
        matrix = glm::rotate(matrix, rotation.x, glm::vec3(1, 0, 0));
        matrix = glm::rotate(matrix, rotation.y, glm::vec3(0, 1, 0));
        matrix = glm::rotate(matrix, rotation.z, glm::vec3(0, 0, 1));
        matrix = glm::scale(matrix, glm::vec3(scale));
        return matrix;
    }
};

struct CameraComponent
{
    float fov   = glm::radians(60.0f);
    float zFar  = 1000.0f;
    float zNear = 0.1f;

    inline constexpr glm::mat4 projectionMatrix(float aspectRatio) const
    {
        return glm::perspective(fov, aspectRatio, zNear, zFar);
    }
};

struct LightComponent
{
    enum class Type : glm::uint8_t
    {
        directional = 0,
        point       = 1
    }
    type = LightComponent::Type::point;
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    float intentsity = 1.0f;
    float attenuation = 0.0f;
};

struct MeshComponent
{
    AssetID id;

    inline operator AssetID& () { return id; }
    inline operator const AssetID& () const { return id; }
};

using ComponentVariant = std::variant<NameComponent, HierarchyComponent, TransformComponent, CameraComponent, LightComponent, MeshComponent>;

} // namespace GE

#endif // COMPONENTS_HPP

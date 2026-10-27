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

#include "Game-Engine/Export.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/TypeList.hpp"
#include "Game-Engine/EntityID.hpp"
#include "Game-Engine/ScriptValues.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <optional>
#include <typeinfo>
#include <string>
#include <map>
#include <memory>
#include <variant>

namespace GE
{

template<typename T>
concept Component = std::is_copy_constructible_v<T> && std::is_move_constructible_v<T> && std::is_destructible_v<T>;

using ComponentID = uint32_t;

static constexpr std::size_t maxComponentTypes = 256;

GE_API ComponentID componentId(const std::type_info&);

template<Component T>
ComponentID componentId()
{
    static ComponentID id = componentId(typeid(T));
    return id;
}

struct NameComponent
{
    std::string name;

    inline operator std::string& () { return name; }
    inline operator const std::string& () const { return name; }
};

struct ParentComponent
{
    std::optional<EntityID> parent;
    std::optional<EntityID> nextChild;
};

struct ChildrenComponent
{
    std::optional<EntityID> firstChild;
};

struct TransformComponent
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale    = glm::vec3(1.0f);

    glm::mat4 worldTransform = 1.0f;

    glm::mat4 localTransform() const
    {
        auto matrix = glm::mat4(1.0f);
        matrix = glm::translate(matrix, position);
        matrix = matrix * glm::mat4_cast(rotation);
        matrix = glm::scale(matrix, glm::vec3(scale));
        return matrix;
    }
};

struct CameraComponent
{
    float fov   = glm::radians(60.0f);
    float zFar  = 1000.0f;
    float zNear = 0.1f;

    inline glm::mat4 projectionMatrix(float aspectRatio) const
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
    AssetID id = 0;

    inline operator AssetID& () { return id; }
    inline operator const AssetID& () const { return id; }
};

class Script;

struct ScriptComponent
{
    std::string name;
    std::map<std::string, VScriptValue> parameters;
    std::shared_ptr<Script> instance;
};

using ECSComponentTypes = TypeList<NameComponent, ParentComponent, ChildrenComponent, TransformComponent, CameraComponent, LightComponent, MeshComponent, ScriptComponent>;
using ComponentVariant = ECSComponentTypes::into<std::variant>;

} // namespace GE

#endif // COMPONENTS_HPP

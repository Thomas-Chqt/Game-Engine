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
#include "Game-Engine/Script.hpp"
#include "Game-Engine/TypeList.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <string_view>
#include <map>
#include <memory>
#include <type_traits>
#include <variant>

#include <yaml-cpp/yaml.h>

namespace GE
{

template<typename T>
struct ECSComponentYamlTraits;

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

struct ScriptComponent
{
    std::string name;
    std::map<std::string, VScriptValue> parameters;
    std::shared_ptr<Script> instance;
};

using ECSComponentTypes = TypeList<NameComponent, HierarchyComponent, TransformComponent, CameraComponent, LightComponent, MeshComponent, ScriptComponent>;
using ComponentVariant = ECSComponentTypes::into<std::variant>;

template<> struct ECSComponentYamlTraits<NameComponent>      { static constexpr std::string_view name = "NameComponent";      };
template<> struct ECSComponentYamlTraits<HierarchyComponent> { static constexpr std::string_view name = "HierarchyComponent"; };
template<> struct ECSComponentYamlTraits<TransformComponent> { static constexpr std::string_view name = "TransformComponent"; };
template<> struct ECSComponentYamlTraits<CameraComponent>    { static constexpr std::string_view name = "CameraComponent";    };
template<> struct ECSComponentYamlTraits<LightComponent>     { static constexpr std::string_view name = "LightComponent";     };
template<> struct ECSComponentYamlTraits<MeshComponent>      { static constexpr std::string_view name = "MeshComponent";      };
template<> struct ECSComponentYamlTraits<ScriptComponent>    { static constexpr std::string_view name = "ScriptComponent";    };

} // namespace GE

namespace YAML
{

template<>
struct convert<glm::vec2>
{
    static Node encode(const glm::vec2& rhs)
    {
        Node node(NodeType::Sequence);
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
    }

    static bool decode(const Node& node, glm::vec2& rhs)
    {
        if (!node.IsSequence() || node.size() != 2)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        return true;
    }
};

template<>
struct convert<glm::vec3>
{
    static Node encode(const glm::vec3& rhs)
    {
        Node node(NodeType::Sequence);
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(const Node& node, glm::vec3& rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};

template<>
struct convert<GE::NameComponent>
{
    static Node encode(const GE::NameComponent& rhs)
    {
        Node node;
        node["name"] = rhs.name;
        return node;
    }

    static bool decode(const Node& node, GE::NameComponent& rhs)
    {
        if (!node.IsMap() || !node["name"])
            return false;

        rhs.name = node["name"].as<std::string>();
        return true;
    }
};

template<>
struct convert<GE::HierarchyComponent>
{
    static Node encode(const GE::HierarchyComponent& rhs)
    {
        Node node;
        node["parent"] = rhs.parent;
        node["firstChild"] = rhs.firstChild;
        node["nextChild"] = rhs.nextChild;
        return node;
    }

    static bool decode(const Node& node, GE::HierarchyComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.parent = node["parent"] ? node["parent"].as<GE::ECSWorld::EntityID>() : INVALID_ENTITY_ID;
        rhs.firstChild = node["firstChild"] ? node["firstChild"].as<GE::ECSWorld::EntityID>() : INVALID_ENTITY_ID;
        rhs.nextChild = node["nextChild"] ? node["nextChild"].as<GE::ECSWorld::EntityID>() : INVALID_ENTITY_ID;
        return true;
    }
};

template<>
struct convert<GE::TransformComponent>
{
    static Node encode(const GE::TransformComponent& rhs)
    {
        Node node;
        node["position"] = rhs.position;
        node["rotation"] = rhs.rotation;
        node["scale"] = rhs.scale;
        return node;
    }

    static bool decode(const Node& node, GE::TransformComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.position = node["position"] ? node["position"].as<glm::vec3>() : glm::vec3{0.0f, 0.0f, 0.0f};
        rhs.rotation = node["rotation"] ? node["rotation"].as<glm::vec3>() : glm::vec3{0.0f, 0.0f, 0.0f};
        rhs.scale = node["scale"] ? node["scale"].as<glm::vec3>() : glm::vec3{1.0f, 1.0f, 1.0f};
        return true;
    }
};

template<>
struct convert<GE::CameraComponent>
{
    static Node encode(const GE::CameraComponent& rhs)
    {
        Node node;
        node["fov"] = rhs.fov;
        node["zFar"] = rhs.zFar;
        node["zNear"] = rhs.zNear;
        return node;
    }

    static bool decode(const Node& node, GE::CameraComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.fov = node["fov"] ? node["fov"].as<float>() : glm::radians(60.0f);
        rhs.zFar = node["zFar"] ? node["zFar"].as<float>() : 1000.0f;
        rhs.zNear = node["zNear"] ? node["zNear"].as<float>() : 0.1f;
        return true;
    }
};

template<>
struct convert<GE::LightComponent::Type>
{
    static Node encode(const GE::LightComponent::Type& rhs)
    {
        switch (rhs)
        {
            case GE::LightComponent::Type::directional: return Node("directional");
            case GE::LightComponent::Type::point: return Node("point");
        }
        return Node();
    }

    static bool decode(const Node& node, GE::LightComponent::Type& rhs)
    {
        if (!node.IsScalar())
            return false;

        const std::string value = node.as<std::string>();
        if (value == "directional")
            rhs = GE::LightComponent::Type::directional;
        else if (value == "point")
            rhs = GE::LightComponent::Type::point;
        else
            return false;

        return true;
    }
};

template<>
struct convert<GE::LightComponent>
{
    static Node encode(const GE::LightComponent& rhs)
    {
        Node node;
        node["type"] = rhs.type;
        node["color"] = rhs.color;
        node["intentsity"] = rhs.intentsity;
        node["attenuation"] = rhs.attenuation;
        return node;
    }

    static bool decode(const Node& node, GE::LightComponent& rhs)
    {
        if (!node.IsMap())
            return false;

        rhs.type = node["type"] ? node["type"].as<GE::LightComponent::Type>() : GE::LightComponent::Type::point;
        rhs.color = node["color"] ? node["color"].as<glm::vec3>() : glm::vec3{1.0f, 1.0f, 1.0f};
        rhs.intentsity = node["intentsity"] ? node["intentsity"].as<float>() : 1.0f;
        rhs.attenuation = node["attenuation"] ? node["attenuation"].as<float>() : 0.0f;
        return true;
    }
};

template<>
struct convert<GE::MeshComponent>
{
    static Node encode(const GE::MeshComponent& rhs)
    {
        Node node;
        node["id"] = rhs.id;
        return node;
    }

    static bool decode(const Node& node, GE::MeshComponent& rhs)
    {
        if (!node.IsMap() || !node["id"])
            return false;

        rhs.id = node["id"].as<GE::AssetID>();
        return true;
    }
};

template<>
struct convert<GE::VScriptValue>
{
    static Node encode(const GE::VScriptValue& rhs)
    {
        Node node;
        std::visit([&](const auto& value) {
            using T = std::remove_cvref_t<decltype(value)>;
            node["type"] = std::string(GE::ScriptValueTraits<T>::name);
            node["data"] = value;
        }, rhs);
        return node;
    }

    static bool decode(const Node& node, GE::VScriptValue& rhs)
    {
        if (!node.IsMap() || !node["type"] || !node["data"])
            return false;

        const std::string type = node["type"].as<std::string>();
        const Node data = node["data"];
        return GE::anyOfType<GE::ScriptValueTypes>([&]<typename ValueT>() {
            if (type != GE::ScriptValueTraits<ValueT>::name)
                return false;
            rhs = data.as<ValueT>();
            return true;
        });
    }
};

template<>
struct convert<GE::ScriptComponent>
{
    static Node encode(const GE::ScriptComponent& rhs)
    {
        Node node;
        node["name"] = rhs.name;
        node["parameters"] = rhs.parameters;
        return node;
    }

    static bool decode(const Node& node, GE::ScriptComponent& rhs)
    {
        if (!node.IsMap() || !node["name"])
            return false;

        rhs.name = node["name"].as<std::string>();
        rhs.parameters = node["parameters"] ? node["parameters"].as<std::map<std::string, GE::VScriptValue>>() : std::map<std::string, GE::VScriptValue>();
        return true;
    }
};

template<>
struct convert<GE::ComponentVariant>
{
    static Node encode(const GE::ComponentVariant& rhs)
    {
        Node node;

        std::visit([&](const auto& component) {
            using ComponentT = std::remove_cvref_t<decltype(component)>;

            node["type"] = std::string(GE::ECSComponentYamlTraits<ComponentT>::name);
            node["data"] = component;
        }, rhs);

        return node;
    }

    static bool decode(const Node& node, GE::ComponentVariant& rhs)
    {
        if (!node.IsMap() || !node["type"] || !node["data"])
            return false;

        const std::string type = node["type"].as<std::string>();
        const Node data = node["data"];
        return GE::anyOfType<GE::ECSComponentTypes>([&]<typename ComponentT>() {
            if (type != GE::ECSComponentYamlTraits<ComponentT>::name)
                return false;
            rhs = data.as<ComponentT>();
            return true;
        });
    }
};

} // namespace YAML

#endif // COMPONENTS_HPP

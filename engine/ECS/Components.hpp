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

#include "AssetManager.hpp"
#include "ECS/ECSWorld.hpp"
#include "Math/Constants.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "Script.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include <nlohmann/json.hpp>

namespace GE
{

struct NameComponent
{
    utils::String name;

    NameComponent() = default;
    NameComponent(const utils::String&);
    inline operator utils::String& () { return name; }
    friend void to_json(nlohmann::json&, const NameComponent&);
    friend void from_json(const nlohmann::json&, NameComponent&);
};

struct HierarchyComponent
{
    ECSWorld::EntityID parent = INVALID_ENTITY_ID;
    ECSWorld::EntityID firstChild = INVALID_ENTITY_ID;
    ECSWorld::EntityID nextChild = INVALID_ENTITY_ID;

    HierarchyComponent() = default;
    friend void to_json(nlohmann::json&, const HierarchyComponent&);
    friend void from_json(const nlohmann::json&, HierarchyComponent&);
};

struct TransformComponent
{
    math::vec3f position = {0.0F, 0.0F, 0.0F};
    math::vec3f rotation = {0.0F, 0.0F, 0.0F};
    math::vec3f scale    = {1.0F, 1.0F, 1.0F};

    TransformComponent() = default;
    TransformComponent(const math::vec3f& position, const math::vec3f& rotation, const math::vec3f& scale);
    inline math::mat4x4 transform() const { return math::mat4x4::translation(position) * math::mat4x4::rotation(rotation) * math::mat4x4::scale(scale); }
    inline operator math::mat4x4 () const { return transform(); }
    friend void to_json(nlohmann::json&, const TransformComponent&);
    friend void from_json(const nlohmann::json&, TransformComponent&);
};

struct CameraComponent
{
    float fov = (float)(60 * (PI / 180.0F));
    float zFar = 10000.0F;
    float zNear = 0.01F;

    CameraComponent() = default;
    CameraComponent(float fov, float zFar, float zNear);
    math::mat4x4 projectionMatrix();
    friend void to_json(nlohmann::json&, const CameraComponent&);
    friend void from_json(const nlohmann::json&, CameraComponent&);
};

struct LightComponent
{
    enum class Type : utils::uint8 { point = 0 } type = LightComponent::Type::point;
    math::rgb color = WHITE3;
    float intentsity = 1.0F;

    LightComponent() = default;
    LightComponent(Type t, math::rgb c, float i);
    friend void to_json(nlohmann::json&, const LightComponent&);
    friend void from_json(const nlohmann::json&, LightComponent&);
};

struct MeshComponent
{
    AssetID assetId;

    MeshComponent() = default;
    MeshComponent(AssetID id);
    inline operator const AssetID& () const { return assetId; }
    inline operator AssetID& () { return assetId; }
    friend void to_json(nlohmann::json&, const MeshComponent&);
    friend void from_json(const nlohmann::json&, MeshComponent&);
};

struct ScriptComponent
{
    utils::String name;
    utils::SharedPtr<Script> instance;

    ScriptComponent() = default;
};

}

#endif // COMPONENTS_HPP
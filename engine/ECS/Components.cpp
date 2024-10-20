/*
 * ---------------------------------------------------
 * Components.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/19 19:05:33
 * ---------------------------------------------------
 */

#include "Components.hpp"
#include "AssetManager.hpp"
#include "ECS/ECSWorld.hpp"
#include "UtilsCPP/String.hpp"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace GE
{

NameComponent::NameComponent(const utils::String& n)
    : name(n)
{
}

void to_json(json& jsn, const NameComponent& comp)
{
    jsn["name"] = comp.name;
}

void from_json(const json& jsn, NameComponent& comp)
{
    auto nameIt = jsn.find("name");
    if (nameIt != jsn.end())
        comp.name = nameIt->template get<std::string>().c_str();
}

void to_json(json& jsn, const HierarchyComponent& comp)
{
    if (comp.parent != INVALID_ENTITY_ID)
        jsn["parent"] = comp.parent;
    if (comp.firstChild != INVALID_ENTITY_ID)
        jsn["firstChild"] = comp.firstChild;
    if (comp.nextChild != INVALID_ENTITY_ID)
        jsn["nextChild"] = comp.nextChild;
}

void from_json(const json& jsn, HierarchyComponent& comp)
{
    auto parentIt = jsn.find("parent");
    if (parentIt != jsn.end())
        comp.parent = parentIt->template get<ECSWorld::EntityID>();

    auto firstChildIt = jsn.find("firstChild");
    if (firstChildIt != jsn.end())
        comp.firstChild = firstChildIt->template get<ECSWorld::EntityID>();

    auto nextChildIt = jsn.find("nextChild");
    if (nextChildIt != jsn.end())
        comp.nextChild = nextChildIt->template get<ECSWorld::EntityID>();
}

TransformComponent::TransformComponent(const math::vec3f& p, const math::vec3f& r, const math::vec3f& s)
    : position(p), rotation(r), scale(s)
{
}

void to_json(json& jsn, const TransformComponent& comp)
{
    jsn = {
        { "position", {
            {"x", comp.position.x },
            {"y", comp.position.y },
            {"z", comp.position.z }
        }},
        { "rotation", {
            {"x", comp.rotation.x },
            {"y", comp.rotation.y },
            {"z", comp.rotation.z }
        }},
        { "scale", {
            {"x", comp.scale.x },
            {"y", comp.scale.y },
            {"z", comp.scale.z }
        }}
    };
}

void from_json(const json& jsn, TransformComponent& comp)
{
    auto positionIt = jsn.find("position");
    if (positionIt != jsn.end())
    {
        auto xIt = positionIt->find("x");
        auto yIt = positionIt->find("y");
        auto zIt = positionIt->find("z");
        if (xIt != positionIt->end() && yIt != positionIt->end() && zIt != positionIt->end())
            comp.position = math::vec3f(xIt->template get<float>(), yIt->template get<float>(), zIt->template get<float>());
    }
    auto rotationIt = jsn.find("rotation");
    if (rotationIt != jsn.end())
    {
        auto xIt = rotationIt->find("x");
        auto yIt = rotationIt->find("y");
        auto zIt = rotationIt->find("z");
        if (xIt != rotationIt->end() && yIt != rotationIt->end() && zIt != rotationIt->end())
            comp.rotation = math::vec3f(xIt->template get<float>(), yIt->template get<float>(), zIt->template get<float>());
    }
    auto scaleIt = jsn.find("scale");
    if (scaleIt != jsn.end())
    {
        auto xIt = scaleIt->find("x");
        auto yIt = scaleIt->find("y");
        auto zIt = scaleIt->find("z");
        if (xIt != scaleIt->end() && yIt != scaleIt->end() && zIt != scaleIt->end())
            comp.scale = math::vec3f(xIt->template get<float>(), yIt->template get<float>(), zIt->template get<float>());
    }
}

CameraComponent::CameraComponent(float f, float zf, float zn)
    : fov(f), zFar(zf), zNear(zn)
{
}

math::mat4x4 CameraComponent::projectionMatrix()
{
    float zs = zFar / (zFar - zNear);
    float ys = 1.0F / std::tan(fov * 0.5F);
    float xs = ys; // (ys / aspectRatio)

    return math::mat4x4(xs,  0,  0,           0,
                         0, ys,  0,           0,
                         0,  0, zs, -zNear * zs,
                         0,  0,  1,           0);
}

void to_json(json& jsn, const CameraComponent& comp)
{
    jsn = {
        { "fov", comp.fov },
        { "zFar", comp.zFar },
        { "zNear", comp.zNear }
    };
}

void from_json(const json& jsn, CameraComponent& comp)
{
    auto fovIt = jsn.find("fov");
    if (fovIt != jsn.end())
        comp.fov = fovIt->template get<float>();
    auto zFarIt = jsn.find("zFar");
    if (zFarIt != jsn.end())
        comp.zFar = zFarIt->template get<float>();
    auto zNearIt = jsn.find("zNear");
    if (zNearIt != jsn.end())
        comp.zNear = zNearIt->template get<float>();
}

LightComponent::LightComponent(Type t, math::rgb c, float i)
    : type(t), color(c), intentsity(i)
{
}

void to_json(json& jsn, const LightComponent& comp)
{
    jsn = {
        { "type", (utils::uint8)comp.type },
        { "color", {
            {"r", comp.color.r },
            {"g", comp.color.g },
            {"b", comp.color.b }
        }},
        { "intentsity", comp.intentsity }
    };
}

void from_json(const json& jsn, LightComponent& comp)
{
    auto typeIt = jsn.find("type");
    if (typeIt != jsn.end())
        comp.type = (LightComponent::Type)(typeIt->template get<utils::uint8>());
    auto colorIt = jsn.find("color");
    if (colorIt != jsn.end())
    {
        auto rIt = colorIt->find("r");
        auto gIt = colorIt->find("g");
        auto bIt = colorIt->find("b");
        if (rIt != colorIt->end() && gIt != colorIt->end() && bIt != colorIt->end())
            comp.color = math::vec3f(rIt->template get<float>(), gIt->template get<float>(), bIt->template get<float>());
    }
    auto intentsityIt = jsn.find("intentsity");
    if (intentsityIt != jsn.end())
        comp.intentsity = intentsityIt->template get<float>();
}

MeshComponent::MeshComponent(AssetID id)
    : assetId(id)
{
}

void to_json(json& jsn, const MeshComponent& comp)
{
    if (comp.assetId.is_nil() == false)
        jsn["assetId"] = uuids::to_string(comp.assetId);
}

void from_json(const json& jsn, MeshComponent& comp)
{
    auto assetIdIt = jsn.find("assetId");
    if (assetIdIt != jsn.end())
        comp.assetId = uuids::uuid::from_string(assetIdIt->template get<std::string>()).value_or(AssetID());
}

}
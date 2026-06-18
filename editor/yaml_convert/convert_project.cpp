#include "convert_project.hpp"
#include "convert_components.hpp" // IWYU pragma: keep
#include "convert_input.hpp"      // IWYU pragma: keep
#include "convert_asset.hpp"      // IWYU pragma: keep
#include "convert_vector.hpp"     // IWYU pragma: keep

#include "Project.hpp"
#include "EditorCamera.hpp"

#include <Game-Engine/Scene.hpp>
#include <Game-Engine/Entity.hpp>

#include <cassert>
#include <ranges>
#include <utility>

namespace YAML
{

template<>
struct convert<GE::Scene::Descriptor>
{
    static Node encode(const GE::Scene::Descriptor& rhs)
    {
        Node node;
        node["name"] = rhs.name;
        node["activeCamera"] = rhs.activeCameraId;

        if (rhs.ecsWorld.entityCount() > 0)
        {
            node["entities"] = rhs.ecsWorld | std::views::transform([&](GE::EntityID id) -> std::pair<GE::EntityID, std::vector<GE::ComponentVariant>> {
                std::vector<GE::ComponentVariant> components;
                GE::forEachType<GE::ECSComponentTypes>([&]<typename ComponentT>() {
                    if (rhs.ecsWorld.template has<ComponentT>(id))
                        components.emplace_back(rhs.ecsWorld.template get<ComponentT>(id));
                });
                return std::pair{id, std::move(components)};
            }) | std::ranges::to<std::vector>();
        }

        return node;
    }

    static bool decode(const Node& node, GE::Scene::Descriptor& rhs)
    {
        if (!node.IsMap() || !node["name"] || !node["activeCamera"])
            return false;

        rhs = GE::Scene::Descriptor{};
        rhs.name = node["name"].as<std::string>();
        rhs.activeCameraId = node["activeCamera"].as<GE::EntityID>();

        if (!node["entities"])
            return true;
        if (!node["entities"].IsSequence())
            return false;

        for (const Node& entityNode : node["entities"])
        {
            const auto [entityId, components] = entityNode.as<std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>>();
            rhs.ecsWorld.registerEntityID(entityId);
            GE::Entity entity{&rhs.ecsWorld, entityId};
            for (const GE::ComponentVariant& component : components)
                std::visit([&]<typename ComponentT>(const ComponentT& value) { entity.emplace<ComponentT>(value); }, component);
        }

        return true;
    }
};

template<>
struct convert<GE_Editor::EditorCamera>
{
    static Node encode(const GE_Editor::EditorCamera& rhs)
    {
        Node node;
        node["position"] = rhs.m_position;
        node["rotation"] = rhs.m_rotation;
        node["fov"] = rhs.m_fov;
        node["zFar"] = rhs.m_zFar;
        node["zNear"] = rhs.m_zNear;
        return node;
    }

    static bool decode(const Node& node, GE_Editor::EditorCamera& rhs)
    {
        if (!node.IsMap())
            return false;
        rhs.m_position = node["position"] ? node["position"].as<glm::vec3>() : glm::vec3{ 0.0f, 0.0f, 0.0f };
        if (const Node rotationNode = node["rotation"])
        {
            rhs.m_rotation = rotationNode["w"]
                ? rotationNode.as<glm::quat>()
                : glm::normalize(glm::quat(rotationNode.as<glm::vec3>()));
        }
        else
        {
            rhs.m_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }
        rhs.m_fov = node["fov"] ? node["fov"].as<float>() : glm::radians(60.0f);
        rhs.m_zFar = node["zFar"] ? node["zFar"].as<float>() : 1000.0f;
        rhs.m_zNear = node["zNear"] ? node["zNear"].as<float>() : 0.1f;
        return true;
    }
};

Node convert<GE_Editor::Project>::encode(const GE_Editor::Project &rhs)
{
    Node node;

    node["name"] = rhs.name;

    if (rhs.resourceDir.has_value())
        node["resourceDir"] = rhs.resourceDir->string();

    if (rhs.scriptLibPath.has_value())
        node["scriptLib"] = std::filesystem::path(*rhs.scriptLibPath).replace_extension().string();

    if (rhs.registeredAssets.empty() == false)
        node["registeredAssets"] = rhs.registeredAssets;

    if (rhs.inputs.empty() == false)
        node["inputs"] = rhs.inputs;

    assert(rhs.scenes.empty() == false);
    node["scenes"] = rhs.scenes;

    node["startScene"] = rhs.startSceneName;

    node["editorCamera"] = rhs.editorCamera;

    if (rhs.editedSceneName.has_value())
        node["editedScene"] = *rhs.editedSceneName;

    if (rhs.selectedEntityId.has_value())
        node["selectedEntity"] = *rhs.selectedEntityId;

    node["imguiSettings"] = rhs.imguiSettings;
    return node;
}

bool convert<GE_Editor::Project>::decode(const Node& node, GE_Editor::Project& rhs)
{
    if (!node.IsMap())
        return false;
    if (!node["name"] || !node["scenes"] || !node["startScene"] || !node["editorCamera"] || !node["imguiSettings"])
        return false;
    if (!node["scenes"] || !node["scenes"].IsSequence() || node["scenes"].size() == 0)
        return false;

    rhs.name = node["name"].as<std::string>();

    rhs.resourceDir = node["resourceDir"] ? std::make_optional(std::filesystem::path(node["resourceDir"].as<std::string>())) : std::nullopt;

    if (node["scriptLib"]) {
        rhs.scriptLibPath = std::filesystem::path(node["scriptLib"].as<std::string>());
        rhs.scriptLibPath->replace_extension(GE_SHARED_LIBRARY_EXTENSION);
    }
    else {
        rhs.scriptLibPath = std::nullopt;
    }

    rhs.registeredAssets = node["registeredAssets"]
        ? node["registeredAssets"].as<std::vector<std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>>>()
        : std::vector<std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>>{};

    rhs.inputs = node["inputs"]
        ? node["inputs"].as<std::vector<std::pair<std::string, GE::VInput>>>()
        : std::vector<std::pair<std::string, GE::VInput>>{};

    rhs.scenes = std::ranges::subrange(node["scenes"].begin(), node["scenes"].end())
        | std::views::transform([](const Node& node){ return node.as<GE::Scene::Descriptor>(); })
        | std::ranges::to<std::vector>();

    rhs.startSceneName = node["startScene"].as<std::string>();

    if (std::ranges::find(rhs.scenes, rhs.startSceneName, &GE::Scene::Descriptor::name) == rhs.scenes.end())
        return false;

    rhs.editorCamera = node["editorCamera"].as<GE_Editor::EditorCamera>();

    rhs.editedSceneName = node["editedScene"]
        ? std::make_optional(node["editedScene"].as<std::string>())
        : std::make_optional(rhs.startSceneName);

    if (rhs.editedSceneName.has_value() && std::ranges::find(rhs.scenes, *rhs.editedSceneName, &GE::Scene::Descriptor::name) == rhs.scenes.end())
        return false;

    rhs.selectedEntityId = node["selectedEntity"]
        ? std::make_optional(node["selectedEntity"].as<GE::EntityID>())
        : std::nullopt;

    rhs.imguiSettings = node["imguiSettings"].as<std::string>();

    return true;
}

}

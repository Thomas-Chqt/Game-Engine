/*
 * ---------------------------------------------------
 * Scene.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 16:52:15
 * ---------------------------------------------------
 */

#ifndef SCENE_HPP
#define SCENE_HPP

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/AssetManagerView.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Export.hpp"

#include "yaml-cpp/yaml.h"

#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace GE
{

class GE_API Scene
{
public:
    struct Descriptor
    {
        std::string name;
        ECSWorld::EntityID activeCamera = INVALID_ENTITY_ID;
        std::vector<std::pair<std::optional<VAssetLocation>, AssetID>> registredAssets;
        std::vector<std::pair<ECSWorld::EntityID, std::vector<ComponentVariant>>> entities;
    };

public:
    Scene() = delete;
    Scene(const Scene&) = delete;
    Scene(Scene&&) = default;

    Scene(AssetManager*, const std::string& name);
    Scene(AssetManager*, const Descriptor&);

    inline auto& ecsWorld(this auto&& self) { return self.m_ecsWorld; }
    inline auto& assetManagerView(this auto&& self) { return self.m_assetManagerView; }

    inline const std::string& name() const { return m_name; }
    inline void setName(const std::string& s) { m_name = s; }

    inline auto activeCamera(this auto&& self) { return basic_entity{&self.m_ecsWorld, self.m_activeCamera}; }
    void setActiveCamera(const Entity& e);

    Entity newEntity(const std::string& name);

    inline void load() { return m_assetManagerView.load(); }
    inline void unload() { m_assetManagerView.unload(); }
    inline bool isLoaded() const { return m_assetManagerView.isLoaded(); }

    Descriptor makeDescriptor() const;

    ~Scene() = default;

private:
    ECSWorld m_ecsWorld;
    AssetManagerView m_assetManagerView;

    std::string m_name;
    ECSWorld::EntityID m_activeCamera = INVALID_ENTITY_ID;

public:
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = default;
};

} // namespace GE

namespace YAML
{

template<>
struct convert<std::pair<std::optional<GE::VAssetLocation>, GE::AssetID>>
{
    static Node encode(const std::pair<std::optional<GE::VAssetLocation>, GE::AssetID>& rhs)
    {
        Node node;
        node["assetId"] = rhs.second;
        if (rhs.first.has_value()) {
            Node assetNode;
            std::visit([&]<GE::ManagableAsset T>(const GE::AssetLocation<T>& assetLocation) {
                assetNode["type"] = std::string(GE::ManagableAssetTraits<T>::name);
                assetNode["containerPath"] = assetLocation.containerPath.string();
                assetNode["index"] = assetLocation.index;
            }, rhs.first.value());
            node["asset"] = assetNode;
        }
        return node;
    }

    static bool decode(const Node& node, std::pair<std::optional<GE::VAssetLocation>, GE::AssetID>& rhs)
    {
        if (!node.IsMap() || !node["assetId"])
            return false;

        rhs.second = node["assetId"].as<GE::AssetID>();
        rhs.first.reset();

        if (!node["asset"])
            return true;

        const Node& assetNode = node["asset"];
        if (!assetNode.IsMap() || !assetNode["type"])
            return false;

        const auto type = assetNode["type"].as<std::string>();
        std::filesystem::path containerPath;
        if (assetNode["containerPath"])
            containerPath = assetNode["containerPath"].as<std::string>();
        if (containerPath.empty())
            return false;
        const uint32_t index = assetNode["index"] ? assetNode["index"].as<uint32_t>() : 0;
        return GE::anyOfType<GE::ManagableAssetTypes>([&]<typename AssetType>() {
            if (type != GE::ManagableAssetTraits<AssetType>::name)
                return false;
            rhs.first = GE::AssetLocation<AssetType>{containerPath, index};
            return true;
        });
    }
};

template<>
struct convert<std::pair<GE::ECSWorld::EntityID, std::vector<GE::ComponentVariant>>>
{
    static Node encode(const std::pair<GE::ECSWorld::EntityID, std::vector<GE::ComponentVariant>>& rhs)
    {
        Node node;
        node["entityId"] = rhs.first;
        node["components"] = rhs.second;
        return node;
    }

    static bool decode(const Node& node, std::pair<GE::ECSWorld::EntityID, std::vector<GE::ComponentVariant>>& rhs)
    {
        if (!node.IsMap() || !node["entityId"] || !node["components"])
            return false;

        rhs.first = node["entityId"].as<GE::ECSWorld::EntityID>();
        rhs.second = node["components"].as<std::vector<GE::ComponentVariant>>();
        return true;
    }
};

template<>
struct convert<GE::Scene::Descriptor>
{
    static Node encode(const GE::Scene::Descriptor& rhs)
    {
        Node node;
        node["name"] = rhs.name;
        node["activeCamera"] = rhs.activeCamera;
        node["registredAssets"] = rhs.registredAssets;
        node["entities"] = rhs.entities;
        return node;
    }

    static bool decode(const Node& node, GE::Scene::Descriptor& rhs)
    {
        if (!node.IsMap() || !node["name"])
            return false;
        rhs.name = node["name"].as<std::string>();
        rhs.activeCamera = node["activeCamera"] ? node["activeCamera"].as<GE::ECSWorld::EntityID>() : INVALID_ENTITY_ID;
        rhs.registredAssets = node["registredAssets"] ? node["registredAssets"].as<std::remove_cvref_t<decltype(rhs.registredAssets)>>() : std::remove_cvref_t<decltype(rhs.registredAssets)>();
        rhs.entities = node["entities"] ? node["entities"].as<std::remove_cvref_t<decltype(rhs.entities)>>() : std::remove_cvref_t<decltype(rhs.entities)>();
        return true;
    }
};

}

#endif // SCENE_HPP

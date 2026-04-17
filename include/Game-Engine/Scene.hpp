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

#include <future>
#include <string>
#include <vector>

namespace GE
{

class Scene
{
public:
    struct Descriptor
    {
        std::string name;
        ECSWorld::EntityID activeCamera = INVALID_ENTITY_ID;
        std::map<AssetID, VAssetPath> registredAssets;
        std::map<ECSWorld::EntityID, std::vector<ComponentVariant>> entities;
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

    inline bool isLoaded() const { return m_assetManagerView.areAllAssetsLoaded(); }
    inline std::future<void> load() const { return m_assetManagerView.loadAllAssets(); }
    inline void unload() { m_assetManagerView.unloadAllAssets(); }

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

#endif // SCENE_HPP

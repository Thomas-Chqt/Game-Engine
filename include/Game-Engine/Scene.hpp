#pragma once

#include "Game-Engine/ECSRegistry.hpp"
#include "Game-Engine/Entity.hpp"

#include <cassert>
#include <future>
#include <optional>
#include <string>
#include <string_view>

namespace GE
{

class AssetManager;

class Scene
{
public:
    struct Descriptor
    {
        std::string name;
        ECSRegistry ecsRegistry;
        std::optional<EntityID> activeCameraId;
    };

public:
    Scene() = delete;
    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;

    Scene(AssetManager*, const Descriptor&);

    const std::string& name() const;
    void setName(const std::string&);

    auto& ecsRegistry(this auto&&);

    auto activeCamera(this auto&& self);
    void setActiveCamera(std::optional<EntityID>);

    std::shared_future<void> loadFuture() const;

    Entity newEntity(std::string_view name);

    ~Scene();

private:
    AssetManager* m_assetManager;
    std::string m_name;
    ECSRegistry m_ecsRegistry;
    std::optional<EntityID> m_activeCameraId;

    std::shared_future<void> m_loadFuture; // the load in the constructor, do not include asset that are added after

public:
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;
};

auto& Scene::ecsRegistry(this auto&& self)
{
    return self.m_ecsRegistry;
}

auto Scene::activeCamera(this auto&& self)
{
    return self.m_activeCameraId.transform([&](EntityID id) {
        return basic_entity{&self.m_ecsRegistry, id};
    });
}

}

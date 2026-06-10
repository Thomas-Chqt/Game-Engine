#pragma once

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Entity.hpp"

#include <cassert>
#include <future>
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
        ECSWorld ecsWorld;
        EntityID activeCameraId;
    };

public:
    Scene() = delete;
    Scene(const Scene&) = delete;
    Scene(Scene&&) = delete;

    Scene(AssetManager*, const Descriptor&);

    const std::string& name() const;
    void setName(const std::string&);

    auto& ecsWorld(this auto&&);

    auto activeCamera(this auto&& self);
    void setActiveCamera(EntityID);

    std::shared_future<void> loadFuture() const;

    Entity newEntity(std::string_view name);

    ~Scene();

private:
    AssetManager* m_assetManager;
    std::string m_name;
    ECSWorld m_ecsWorld;
    EntityID m_activeCameraId;

    std::shared_future<void> m_loadFuture; // the load in the constructor, do not include asset that are added after

public:
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;
};

auto& Scene::ecsWorld(this auto&& self)
{
    return self.m_ecsWorld;
}

auto Scene::activeCamera(this auto&& self)
{
    assert(self.m_ecsWorld.isValidEntityID(self.m_activeCameraId));
    return basic_entity{&self.m_ecsWorld, self.m_activeCameraId};
}

}

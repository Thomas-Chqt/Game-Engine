#include "Game-Engine/Scene.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/ECSRegistry.hpp"
#include "Game-Engine/Entity.hpp"

#include <cassert>
#include <string>
#include <string_view>

namespace GE
{

Scene::Scene(AssetManager* assetManager, const Descriptor& descriptor)
    : m_assetManager(assetManager)
    , m_name(descriptor.name)
    , m_ecsRegistry(descriptor.ecsRegistry)
    , m_activeCameraId(descriptor.activeCameraId)
{
    assert(m_assetManager);
    auto assetIds = m_ecsRegistry
        | ECSView<MeshComponent>()
        | std::views::transform([](const auto& e){
            auto& [mesh] = e;
            return mesh.id;
        });
    m_loadFuture = assetManager->loadAssets(assetIds).share();
}

const std::string& Scene::name() const
{
    return m_name;;
}

void Scene::setName(const std::string& name)
{
    m_name = name;
}

void Scene::setActiveCamera(std::optional<EntityID> id)
{
    assert(!id || m_ecsRegistry.isValidEntityID(*id));
    m_activeCameraId = id;
}

std::shared_future<void> Scene::loadFuture() const
{
    return m_loadFuture;
}

Entity Scene::newEntity(std::string_view name)
{
    Entity newEntity(&m_ecsRegistry, m_ecsRegistry.newEntityID());
    newEntity.emplace<NameComponent>(std::string(name));
    return newEntity;
}

Scene::~Scene()
{
    assert(m_assetManager);
    auto assetIds = m_ecsRegistry
        | ECSView<MeshComponent>()
        | std::views::transform([](const auto& e){
            auto& [mesh] = e;
            return mesh.id;
        });
    m_assetManager->unloadAssets(assetIds);
}

}

/*
 * ---------------------------------------------------
 * Scene.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:10:58
 * ---------------------------------------------------
 */

#include "Game-Engine/Scene.hpp"

#include "Game-Engine/Components.hpp"

#include <type_traits>
#include <variant>

namespace GE
{

Scene::Scene(AssetManager* assetManager, const std::string& name)
    : m_assetManagerView(assetManager)
    , m_name(name)
{
}

Scene::Scene(AssetManager* assetManager, const Descriptor& desc)
    : m_assetManagerView(assetManager, desc.registredAssets)
    , m_name(desc.name)
    , m_activeCamera(desc.activeCamera)
{
    for (auto& [id, vComponents] : desc.entities) {
        m_ecsWorld.registerEntityID(id);
        for (auto& vComponent : vComponents) {
            std::visit([&](auto& component) {
                m_ecsWorld.emplace<std::remove_cvref_t<decltype(component)>>(id, component);
            }, vComponent);
        }
    }
}

void Scene::setActiveCamera(const Entity& e)
{
    assert(e.world == &m_ecsWorld);
    m_activeCamera = e.entityId;
}

Entity Scene::newEntity(const std::string& name)
{
    Entity newEntity(&m_ecsWorld, m_ecsWorld.newEntityID());
    newEntity.emplace<NameComponent>(name);
    return newEntity;
}

Scene::Descriptor Scene::makeDescriptor() const
{
    Scene::Descriptor desc;
    desc.name = m_name;
    desc.activeCamera = m_activeCamera;
    desc.registredAssets = m_assetManagerView.registredAssets();

    for (ECSWorld::EntityID entityId : m_ecsWorld)
    {
        std::vector<ComponentVariant> components;
        const_Entity entity{&m_ecsWorld, entityId};

        forEachECSComponentType([&]<typename ComponentT>() {
            if (!entity.has<ComponentT>())
                return;

            if constexpr (std::is_same_v<ComponentT, ScriptComponent>) {
                ScriptComponent scriptComponent = entity.get<ScriptComponent>();
                // TODO find a solution to not have to do that
                scriptComponent.instance.reset();
                components.emplace_back(std::move(scriptComponent));
            } else
                components.emplace_back(entity.get<ComponentT>());
        });

        desc.entities.emplace(entityId, std::move(components));
    }

    return desc;
}

}

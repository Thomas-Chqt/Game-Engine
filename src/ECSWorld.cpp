/*
 * ---------------------------------------------------
 * ECSWorld.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 09:35:25
 * ---------------------------------------------------
 */

#include "ECS/ECSWorld.hpp"
#include "ECS/Components.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Types.hpp"
#include <cassert>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace GE
{

ECSWorld::ECSWorld()
{
    m_archetypes.insert(ArchetypeID{0}, Archetype());
}

ECSWorld::EntityID ECSWorld::newEntityID()
{
    // new entity has no component so directly inserting in empty archetype (the one with only the entity id)
    ArchetypeID newEntityArcId = { 0 };
    Archetype& newEntityArchetype = m_archetypes[newEntityArcId];
    utils::uint64 newEntityIdx = newEntityArchetype.allocateCollum();

    EntityID newEntityId;
    if (m_availableEntityIDs.isEmpty())
    {
        newEntityId = m_entityDatas.length();
        m_entityDatas.append(EntityData{newEntityArcId, newEntityIdx});
    }
    else
    {
        newEntityId = m_availableEntityIDs.pop(m_availableEntityIDs.begin());
        m_entityDatas[newEntityId] = EntityData{newEntityArcId, newEntityIdx};
    }
    newEntityArchetype.getEntityID(newEntityIdx) = newEntityId;

    assert(isValidEntityID(newEntityId));
    return newEntityId;
}

void ECSWorld::deleteEntityID(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    Archetype& entityArch = m_archetypes[m_entityDatas[entityId].archetypeId];
    utils::uint64 entityIdx = m_entityDatas[entityId].idx;

    // last entity of the archetype will be move to the index of the delete entity
    // so the idx in the entity datas need to be change
    m_entityDatas[entityArch.getEntityID(entityArch.size() - 1)].idx = entityIdx;

    if (entityIdx != entityArch.size() -1)
    {
        entityArch.destructCollum(entityIdx);
        Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityIdx);
    }
    entityArch.destructCollum(entityArch.size() - 1);
    entityArch.freeLastCollum();

    m_availableEntityIDs.insert(entityId);
}

utils::uint32 ECSWorld::componentCount()
{
    utils::uint64 count = 0;
    for (auto& [id, arch] : m_archetypes)
        count += (id.size() - 1) * arch.size();
    return count;
}

ECSWorld::Iterator ECSWorld::begin() const
{
    EntityID id = 0;
    while (m_availableEntityIDs.contain(id))
        id++;
    return Iterator(*this, id);
}

ECSWorld::Iterator ECSWorld::end() const
{
    return Iterator(*this, m_entityDatas.length());
}

ECSWorld::ComponentID ECSWorld::nextComponentID()
{
    // start at 1 because 0 is reserved for the entity id
    static ComponentID id = 1;
    return id++;
};

void to_json(nlohmann::json& jsn, const ECSWorld& world)
{
    jsn["entities"] = json::array();

    for (auto id : world)
    {
        json entityJsn;
        entityJsn["id"] = id;

        if (world.has<NameComponent>(id))
            entityJsn["nameComponent"] = world.get<NameComponent>(id);

        if (world.has<HierarchyComponent>(id))
            entityJsn["hierarchyComponent"] = world.get<HierarchyComponent>(id);

        if (world.has<TransformComponent>(id))
            entityJsn["transformComponent"] = world.get<TransformComponent>(id);

        if (world.has<CameraComponent>(id))
            entityJsn["cameraComponent"] = world.get<CameraComponent>(id);

        if (world.has<LightComponent>(id))
            entityJsn["lightComponent"] = world.get<LightComponent>(id);

        if (world.has<MeshComponent>(id))
            entityJsn["meshComponent"] = world.get<MeshComponent>(id);

        if (world.has<ScriptComponent>(id))
            entityJsn["scriptComponent"] = world.get<ScriptComponent>(id);

        jsn["entities"].emplace_back(entityJsn);
    }

    jsn["availableEntityID"] = json::array();

    for (auto id : world.m_availableEntityIDs)
        jsn["availableEntityID"].emplace_back(id);
}

void from_json(const nlohmann::json& jsn, ECSWorld& world)
{
    auto entitiesIt = jsn.find("entities");
    auto availableEntityIDIt = jsn.find("availableEntityID");
    utils::uint64 entityDatasCount = 0;
    if (entitiesIt != jsn.end())
        entityDatasCount += jsn["entities"].size();
    if (availableEntityIDIt != jsn.end())
        entityDatasCount += jsn["availableEntityID"].size();

    world.m_entityDatas = utils::Array<ECSWorld::EntityData>(entityDatasCount);

    for (auto& entity : jsn["entities"])
    {
        ECSWorld::EntityID entityId = entity["id"].template get<ECSWorld::EntityID>();
        ECSWorld::ArchetypeID newEntityArcId = { 0 };
        ECSWorld::Archetype& newEntityArchetype = world.m_archetypes[newEntityArcId];
        utils::uint64 newEntityIdx = newEntityArchetype.allocateCollum();
        world.m_entityDatas[entityId] = ECSWorld::EntityData{newEntityArcId, newEntityIdx};
        newEntityArchetype.getEntityID(newEntityIdx) = entityId;
        
        auto nameComponentIt = entity.find("nameComponent");
        if (nameComponentIt != entity.end())
            world.emplace<NameComponent>(entityId) = nameComponentIt->template get<NameComponent>();

        auto hierarchyComponentIt = entity.find("hierarchyComponent");
        if (hierarchyComponentIt != entity.end())
            world.emplace<HierarchyComponent>(entityId) = hierarchyComponentIt->template get<HierarchyComponent>();

        auto transformComponentIt = entity.find("transformComponent");
        if (transformComponentIt != entity.end())
            world.emplace<TransformComponent>(entityId) = transformComponentIt->template get<TransformComponent>();

        auto cameraComponentIt = entity.find("cameraComponent");
        if (cameraComponentIt != entity.end())
            world.emplace<CameraComponent>(entityId) = cameraComponentIt->template get<CameraComponent>();

        auto lightComponentIt = entity.find("lightComponent");
        if (lightComponentIt != entity.end())
            world.emplace<LightComponent>(entityId) = lightComponentIt->template get<LightComponent>();

        auto meshComponenIt = entity.find("meshComponent");
        if (meshComponenIt != entity.end())
            world.emplace<MeshComponent>(entityId) = meshComponenIt->template get<MeshComponent>();

        auto scriptComponentIt = entity.find("scriptComponent");
        if (scriptComponentIt != entity.end())
            world.emplace<ScriptComponent>(entityId) = scriptComponentIt->template get<ScriptComponent>();
    }

    for (auto& id : jsn["availableEntityID"])
        world.m_availableEntityIDs.insert(id);
}

}
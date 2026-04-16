/*
 * ---------------------------------------------------
 * ECSWorld.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 09:35:25
 * ---------------------------------------------------
 */

#include "Game-Engine/ECSWorld.hpp"

#include <cassert>
#include <cstddef>
#include <ranges>

// using json = nlohmann::json;

namespace GE
{

ECSWorld::ECSWorld()
{
    m_archetypes.insert(std::make_pair(ArchetypeID{0}, Archetype()));
}

ECSWorld::EntityID ECSWorld::newEntityID()
{
    EntityID newEntityId;
    if (m_availableEntityIDs.empty())
        newEntityId = m_entityDatas.size();
    else
        newEntityId = *m_availableEntityIDs.begin();
    registerEntityID(newEntityId);
    return newEntityId;
}

void ECSWorld::registerEntityID(ECSWorld::EntityID id)
{
    assert(isValidEntityID(id) == false); // user is responsible to be sure the id is not already used

    // new entity has no component so directly inserting in empty archetype (the one with only the entity id)
    ArchetypeID newEntityArcId = { 0 };
    Archetype& newEntityArchetype = m_archetypes[newEntityArcId];
    uint64_t newEntityIdx = newEntityArchetype.allocateCollum();
    auto it = m_availableEntityIDs.find(id);
    if (it != m_availableEntityIDs.end())
    {
        m_availableEntityIDs.erase(it);
        m_entityDatas[id] = EntityData{newEntityArcId, newEntityIdx};
    }
    else
    {
        if (id > m_entityDatas.size()) {
            for (auto availableID : std::views::iota(m_entityDatas.size(), id))
                m_availableEntityIDs.insert(availableID);
            m_entityDatas.resize(id);
        }
        m_entityDatas.push_back(EntityData{newEntityArcId, newEntityIdx});
    }
    newEntityArchetype.getEntityID(newEntityIdx) = id;
    assert(isValidEntityID(id));
}

void ECSWorld::deleteEntityID(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    Archetype& entityArch = m_archetypes[m_entityDatas[entityId].archetypeId];
    uint64_t entityIdx = m_entityDatas[entityId].idx;

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

uint32_t ECSWorld::componentCount()
{
    uint64_t count = 0;
    for (auto& [id, arch] : m_archetypes)
        count += (id.size() - 1) * arch.size();
    return count;
}

ECSWorld::Iterator ECSWorld::begin() const
{
    EntityID id = 0;
    while (m_availableEntityIDs.contains(id))
        id++;
    return Iterator(*this, id);
}

ECSWorld::Iterator ECSWorld::end() const
{
    return Iterator(*this, m_entityDatas.size());
}

ECSWorld::ComponentID ECSWorld::nextComponentID()
{
    // start at 1 because 0 is reserved for the entity id
    static ComponentID id = 1;
    return id++;
};

// void to_json(nlohmann::json& jsn, const ECSWorld& world)
// {
//     jsn["entities"] = json::array();

//     for (auto id : world)
//     {
//         json entityJsn;
//         entityJsn["id"] = id;

//         if (world.has<NameComponent>(id))
//             entityJsn["nameComponent"] = world.get<NameComponent>(id);

//         if (world.has<HierarchyComponent>(id))
//             entityJsn["hierarchyComponent"] = world.get<HierarchyComponent>(id);

//         if (world.has<TransformComponent>(id))
//             entityJsn["transformComponent"] = world.get<TransformComponent>(id);

//         if (world.has<CameraComponent>(id))
//             entityJsn["cameraComponent"] = world.get<CameraComponent>(id);

//         if (world.has<LightComponent>(id))
//             entityJsn["lightComponent"] = world.get<LightComponent>(id);

//         if (world.has<MeshComponent>(id))
//             entityJsn["meshComponent"] = world.get<MeshComponent>(id);

//         if (world.has<ScriptComponent>(id))
//             entityJsn["scriptComponent"] = world.get<ScriptComponent>(id);

//         jsn["entities"].emplace_back(entityJsn);
//     }

//     jsn["availableEntityID"] = json::array();

//     for (auto id : world.m_availableEntityIDs)
//         jsn["availableEntityID"].emplace_back(id);
// }

// void from_json(const nlohmann::json& jsn, ECSWorld& world)
// {
//     auto entitiesIt = jsn.find("entities");
//     auto availableEntityIDIt = jsn.find("availableEntityID");
//     uint64_t entityDatasCount = 0;
//     if (entitiesIt != jsn.end())
//         entityDatasCount += jsn["entities"].size();
//     if (availableEntityIDIt != jsn.end())
//         entityDatasCount += jsn["availableEntityID"].size();

//     world.m_entityDatas = utils::Array<ECSWorld::EntityData>(entityDatasCount);

//     for (auto& entity : jsn["entities"])
//     {
//         ECSWorld::EntityID entityId = entity["id"].template get<ECSWorld::EntityID>();
//         ECSWorld::ArchetypeID newEntityArcId = { 0 };
//         ECSWorld::Archetype& newEntityArchetype = world.m_archetypes[newEntityArcId];
//         uint64_t newEntityIdx = newEntityArchetype.allocateCollum();
//         world.m_entityDatas[entityId] = ECSWorld::EntityData{newEntityArcId, newEntityIdx};
//         newEntityArchetype.getEntityID(newEntityIdx) = entityId;

//         auto nameComponentIt = entity.find("nameComponent");
//         if (nameComponentIt != entity.end())
//             world.emplace<NameComponent>(entityId) = nameComponentIt->template get<NameComponent>();

//         auto hierarchyComponentIt = entity.find("hierarchyComponent");
//         if (hierarchyComponentIt != entity.end())
//             world.emplace<HierarchyComponent>(entityId) = hierarchyComponentIt->template get<HierarchyComponent>();

//         auto transformComponentIt = entity.find("transformComponent");
//         if (transformComponentIt != entity.end())
//             world.emplace<TransformComponent>(entityId) = transformComponentIt->template get<TransformComponent>();

//         auto cameraComponentIt = entity.find("cameraComponent");
//         if (cameraComponentIt != entity.end())
//             world.emplace<CameraComponent>(entityId) = cameraComponentIt->template get<CameraComponent>();

//         auto lightComponentIt = entity.find("lightComponent");
//         if (lightComponentIt != entity.end())
//             world.emplace<LightComponent>(entityId) = lightComponentIt->template get<LightComponent>();

//         auto meshComponenIt = entity.find("meshComponent");
//         if (meshComponenIt != entity.end())
//             world.emplace<MeshComponent>(entityId) = meshComponenIt->template get<MeshComponent>();

//         auto scriptComponentIt = entity.find("scriptComponent");
//         if (scriptComponentIt != entity.end())
//             world.emplace<ScriptComponent>(entityId) = scriptComponentIt->template get<ScriptComponent>();
//     }

//     for (auto& id : jsn["availableEntityID"])
//         world.m_availableEntityIDs.insert(id);
// }

}

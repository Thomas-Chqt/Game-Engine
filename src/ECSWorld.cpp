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
#include <mutex>
#include <ranges>
#include <string>
#include <typeinfo>

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

ECSWorld::ComponentID ECSWorld::nextComponentID()
{
    // start at 1 because 0 is reserved for the entity id
    static ComponentID id = 1;
    return id++;
};

ECSWorld::ComponentID ECSWorld::componentID(const std::type_info& typeInfo)
{
    static std::mutex mutex;
    static std::map<std::string, ComponentID> componentIDs;

    std::lock_guard lock(mutex);

    auto [it, inserted] = componentIDs.emplace(typeInfo.name(), 0);
    if (inserted)
        it->second = nextComponentID();

    return it->second;
}

}

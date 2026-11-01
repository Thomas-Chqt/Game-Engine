#include "Game-Engine/ECSRegistry.hpp"

#include "Game-Engine/Archetype.hpp"
#include "Game-Engine/EntityID.hpp"

#include <uuid.h>

#include <cassert>
#include <cstddef>

namespace GE
{

EntityID ECSRegistry::newEntityID()
{
    EntityID newEntityId = uuids::uuid_system_generator{}();
    assert(m_entityArchetypes.contains(newEntityId) == false);
    registerEntityID(newEntityId);
    return newEntityId;
}

void ECSRegistry::registerEntityID(EntityID id)
{
    assert(!id.is_nil());
    assert(m_entityArchetypes.contains(id) == false);
    // entities always have the EntityID as first component
    ArchetypeID newEntityArcId = archetypeId<EntityID>();
    m_entityArchetypes.emplace(id, newEntityArcId);
    auto it = m_archetypes.find(newEntityArcId);
    if (it == m_archetypes.end()) {
        [[maybe_unused]] auto [insertId, inserted] = m_archetypes.emplace(newEntityArcId, Archetype::create<EntityID>());
        assert(inserted);
        it = insertId;
    }
    it->second.emplaceBack<EntityID>(id, id);
}

void ECSRegistry::deleteEntityID(EntityID entityId)
{
    assert(m_entityArchetypes.contains(entityId));
    Archetype& entityArch = m_archetypes.at(m_entityArchetypes.at(entityId));
    entityArch.erase(entityId);
    m_entityArchetypes.erase(entityId);
}

bool ECSRegistry::isValidEntityID(EntityID id) const
{
    return m_entityArchetypes.contains(id);
}

size_t ECSRegistry::archetypeCount() const
{
    return m_archetypes.size();
}

size_t ECSRegistry::entityCount() const
{
    return m_entityArchetypes.size();
}

size_t ECSRegistry::componentCount() const
{
    size_t count = 0;
    for (const auto& [id, arch] : m_archetypes)
        count += (id.count() - 1) * arch.size(); // EntityID is registry bookkeeping.
    return count;
}

ECSRegistry::iterator ECSRegistry::begin() const
{
    return {m_entityArchetypes.begin()};
}

ECSRegistry::iterator ECSRegistry::end() const
{
    return {m_entityArchetypes.end()};
}

}

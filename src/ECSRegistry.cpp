#include "Game-Engine/ECSRegistry.hpp"

#include "Game-Engine/Archetype.hpp"
#include "Game-Engine/EntityID.hpp"

#include <uuid.h>

#include <cassert>
#include <cstddef>
#include <vector>

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

EntityIDMap ECSRegistry::insert(const ECSRegistry& source)
{
    assert(&source != this);
    EntityIDMap ids;
    ids.reserve(source.entityCount());
    for (const auto& [archetypeId, archetype] : source.m_archetypes) {
        for (const auto id : archetype.getComponents<EntityID>()) {
            const EntityID newId = uuids::uuid_system_generator{}();
            assert(isValidEntityID(newId) == false);
            ids.emplace(id, newId);
        }
    }
    if (ids.empty())
        return ids;

    std::vector<std::pair<ArchetypeID, size_t>> appended;
    appended.reserve(source.archetypeCount());
    m_entityArchetypes.reserve(entityCount() + source.entityCount());
    try {
        for (const auto& [archetypeId, sourceArchetype] : source.m_archetypes) {
            if (sourceArchetype.size() == 0)
                continue;
            auto it = m_archetypes.find(archetypeId);
            if (it == m_archetypes.end())
                it = m_archetypes.emplace(archetypeId, sourceArchetype.createEmpty()).first;
            appended.emplace_back(archetypeId, it->second.size());
            it->second.appendCopies(sourceArchetype, ids);
        }
        for (const auto& [archetypeId, originalSize] : appended) {
            for (const auto id : m_archetypes.at(archetypeId).getComponents<EntityID>().subspan(originalSize)) {
                [[maybe_unused]] const auto [it, inserted] = m_entityArchetypes.emplace(id, archetypeId);
                assert(inserted);
            }
        }
    }
    catch (...) {
        for (const auto& [archetypeId, originalSize] : appended) {
            auto& archetype = m_archetypes.at(archetypeId);
            for (const auto id : archetype.getComponents<EntityID>().subspan(originalSize))
                m_entityArchetypes.erase(id);
            archetype.truncate(originalSize);
        }
        throw;
    }
    return ids;
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

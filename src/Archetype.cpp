#include "Game-Engine/Archetype.hpp"

namespace GE
{

Archetype::Archetype(const Archetype& other)
    : m_archetypeId(other.m_archetypeId)
    , m_entityIndices(other.m_entityIndices)
{
    for (const auto& [id, buffer] : other.m_componentBuffers)
        m_componentBuffers.emplace(id, buffer->clone());
}

size_t Archetype::size() const
{
    assert(std::ranges::all_of(m_componentBuffers, [&](const auto& pair) {
        return pair.second->size() == m_entityIndices.size();
    }));
    return m_entityIndices.size();
}

void Archetype::erase(EntityID id)
{
    assert(m_entityIndices.contains(id));
    const auto idx = m_entityIndices.at(id);
    const auto last = size() - 1;
    const auto lastId = static_cast<const ComponentVector<EntityID>&>(*m_componentBuffers.at(componentId<EntityID>())).vector().back();
    for (auto& [compId, buffer] : m_componentBuffers) {
        buffer->swapComponents(idx, last);
        buffer->eraseLast();
    }
    if (idx != last)
        m_entityIndices.at(lastId) = idx;
    m_entityIndices.erase(id);
}

void Archetype::moveEntity(EntityID id, Archetype& dst)
{
    assert(&dst != this);
    assert(m_entityIndices.contains(id));
    assert(!dst.m_entityIndices.contains(id));
    assert((m_archetypeId & dst.m_archetypeId) == dst.m_archetypeId);
    const auto idx = m_entityIndices.at(id);
    const auto last = size() - 1;
    const auto lastId = static_cast<const ComponentVector<EntityID>&>(*m_componentBuffers.at(componentId<EntityID>())).vector().back();
    dst.m_entityIndices.emplace(id, dst.size());
    for (auto& [compId, buffer] : m_componentBuffers) {
        if (auto it = dst.m_componentBuffers.find(compId); it != dst.m_componentBuffers.end())
            buffer->moveComponent(idx, *it->second);
        else {
            buffer->swapComponents(idx, last);
            buffer->eraseLast();
        }
    }
    if (idx != last)
        m_entityIndices.at(lastId) = idx;
    m_entityIndices.erase(id);
}

Archetype::Archetype(std::unordered_map<ComponentID, std::unique_ptr<AComponentVector>> buffers)
    : m_componentBuffers(std::move(buffers))
{
    for (const auto& [id, buffer] : m_componentBuffers) {
        assert(buffer->size() == 0);
        m_archetypeId.set(id);
    }
}

Archetype& Archetype::operator=(const Archetype& other)
{
    if (this != &other)
        *this = Archetype(other);
    return *this;
}

}

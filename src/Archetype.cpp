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

Archetype Archetype::createEmpty() const
{
    std::unordered_map<ComponentID, std::unique_ptr<AComponentVector>> buffers;
    for (const auto& [id, buffer] : m_componentBuffers)
        buffers.emplace(id, buffer->create());
    return {std::move(buffers)};
}

void Archetype::appendCopies(const Archetype& source, const EntityIDMap& ids)
{
    assert(&source != this);
    assert(source.m_archetypeId == m_archetypeId);
    const auto originalSize = size();
    m_entityIndices.reserve(originalSize + source.size());
    try {
        for (const auto& [componentId, buffer] : source.m_componentBuffers)
            buffer->copyComponents(*m_componentBuffers.at(componentId), ids);
        const auto entityIds = getComponents<EntityID>();
        for (size_t idx = originalSize; idx < entityIds.size(); ++idx) {
            [[maybe_unused]] const auto [it, inserted] = m_entityIndices.emplace(entityIds[idx], idx);
            assert(inserted);
        }
    }
    catch (...) {
        truncate(originalSize);
        throw;
    }
}

void Archetype::truncate(size_t newSize)
{
    assert(newSize <= getComponents<EntityID>().size());
    for (const auto id : getComponents<EntityID>().subspan(newSize))
        m_entityIndices.erase(id);
    for (auto& [componentId, buffer] : m_componentBuffers)
        buffer->truncate(newSize);
}

void Archetype::erase(EntityID id)
{
    assert(m_entityIndices.contains(id));
    const auto idx = m_entityIndices.at(id);
    const auto last = size() - 1;
    const auto lastId = getComponents<EntityID>().back();
    for (auto& [compId, buffer] : m_componentBuffers) {
        buffer->swapComponents(idx, last);
        buffer->eraseLast();
    }
    if (idx != last)
        m_entityIndices.at(lastId) = idx;
    m_entityIndices.erase(id);
}

void moveEntity(EntityID id, Archetype& src, Archetype& dst)
{
    assert(&src != &dst);
    assert(src.m_entityIndices.contains(id));
    assert(!dst.m_entityIndices.contains(id));
    assert((src.m_archetypeId & dst.m_archetypeId) == dst.m_archetypeId);
    const auto idx = src.m_entityIndices.at(id);
    const auto last = src.size() - 1;
    const auto lastId = src.getComponents<EntityID>().back();
    const auto dstSize = dst.size();
    for (auto& [compId, buffer] : dst.m_componentBuffers)
        buffer->reserveForAppend();
    dst.m_entityIndices.emplace(id, dstSize);
    // Allocation is complete and component moves cannot throw.
    for (auto& [compId, buffer] : src.m_componentBuffers) {
        if (auto it = dst.m_componentBuffers.find(compId); it != dst.m_componentBuffers.end())
            buffer->moveComponent(idx, *it->second);
        else {
            buffer->swapComponents(idx, last);
            buffer->eraseLast();
        }
    }
    if (idx != last)
        src.m_entityIndices.at(lastId) = idx;
    src.m_entityIndices.erase(id);
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

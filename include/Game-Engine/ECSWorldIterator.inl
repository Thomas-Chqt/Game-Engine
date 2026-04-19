/*
 * ---------------------------------------------------
 * ECSWorldIterator.inl
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

class Iterator
{
private:
    friend class ECSWorld;

public:
    Iterator() = default;
    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) = default;

    ~Iterator() = default;

private:
    Iterator(const ECSWorld* world, const std::vector<EntityData>::const_iterator& entityDataIt)
        : m_world(world),
          m_entityDataIt(entityDataIt)
    {
        assert(m_world);
    }

    const ECSWorld* m_world = nullptr;
    std::vector<EntityData>::const_iterator m_entityDataIt;

public:
    Iterator& operator=(const Iterator& cp) = default;
    Iterator& operator=(Iterator&& mv) = default;

    inline EntityID operator*() const
    {
        const Archetype& archetype = m_world->m_archetypes.at(m_entityDataIt->archetypeId);
        return archetype.getEntityID(m_entityDataIt->idx);
    }

    inline Iterator& operator++() { return m_entityDataIt++, *this; }
    inline void operator++(int) { ++(*this); }

    inline bool operator==(std::default_sentinel_t) const { return m_entityDataIt == m_world->m_entityDatas.end(); }
};

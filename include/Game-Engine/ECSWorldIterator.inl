/*
 * ---------------------------------------------------
 * ECSWorldIterator.inl
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

struct Iterator
{
private:
    friend class ECSWorld;

public:
    using iterator_concept = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = EntityID;

    Iterator() = default;
    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) = default;

    ~Iterator() = default;

private:
    Iterator(const ECSWorld* world, EntityID id)
        : m_world(world), m_id(id)
    {
        assert(m_world);
    }

    const ECSWorld* m_world = nullptr;
    EntityID m_id = INVALID_ENTITY_ID;

public:
    Iterator& operator=(const Iterator& cp) = default;
    Iterator& operator=(Iterator&& mv) = default;

    inline EntityID operator*() const { return m_id; }

    inline Iterator& operator++()
    {
        do ++m_id;
        while (m_world->m_availableEntityIDs.contains(m_id));
        return *this;
    }

    inline void operator++(int) { ++(*this); }

    inline bool operator==(std::default_sentinel_t) const { return m_id == m_world->m_entityDatas.size(); }
};

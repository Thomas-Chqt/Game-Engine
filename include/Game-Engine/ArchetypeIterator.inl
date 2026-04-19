/*
 * ---------------------------------------------------
 * ArchetypeIterator.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

class Iterator
{
public:
    friend class Archetype;

public:
    Iterator() = default;
    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) = default;

    inline uint64_t idx() const { return m_idx; }

    ~Iterator() = default;

private:
    Iterator(const Archetype* archetype, uint64_t idx)
        : m_archetype(archetype)
        , m_idx(idx)
    {
        assert(m_archetype);
    }

    const Archetype* m_archetype = nullptr;
    uint64_t m_idx = 0;

public:
    Iterator& operator=(const Iterator& cp) = default;
    Iterator& operator=(Iterator&& mv) = default;

    inline EntityID operator*() const { return m_archetype->getEntityID(m_idx); }

    inline Iterator& operator++() { return m_idx++, *this; }
    inline void operator++(int) { ++(*this); }

    inline bool operator==(std::default_sentinel_t) const { return m_idx == m_archetype->m_size; }
};

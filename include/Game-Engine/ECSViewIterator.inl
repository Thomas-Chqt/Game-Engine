/*
 * ---------------------------------------------------
 * ECSViewIterator.inl
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

class Iterator
{
public:
    using iterator_concept = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = ecs_view_item<ECSWorldT, Cs...>;

private:
    friend class basic_ecsView;
    using ArchetypeMapIterator = decltype(std::declval<ECSWorldT&>().m_archetypes.begin());

public:
    Iterator() = default;
    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) = default;

    ~Iterator() = default;

private:
    Iterator(ECSWorldT* world, const Predicate& predicate, const ArchetypeMapIterator& archetypeIt, const typename ECSWorldT::Archetype::Iterator& entityIt)
        : m_world(world)
        , m_predicate(predicate)
        , m_archetypeIt(archetypeIt)
        , m_entityIt(entityIt)
    {
    }

    ECSWorldT* m_world = nullptr;
    Predicate m_predicate;
    ArchetypeMapIterator m_archetypeIt;
    typename ECSWorldT::Archetype::Iterator m_entityIt;

public:
    Iterator& operator=(const Iterator& cp) = default;
    Iterator& operator=(Iterator&& mv) = default;

    inline value_type operator*() const
    {
        return value_type{
            .entityId = *m_entityIt,
            .components = {*m_archetypeIt->second.template getComponentPointer<Cs>(m_entityIt.idx())...}
        };
    }

    inline Iterator& operator++()
    {
        ++m_entityIt;
        if (m_entityIt == m_archetypeIt->second.end())
        {
            do ++m_archetypeIt;
            while (m_archetypeIt != m_world->m_archetypes.end() && (std::ranges::includes(m_archetypeIt->first, m_predicate) == false || m_archetypeIt->second.size() == 0));

            if (m_archetypeIt != m_world->m_archetypes.end())
                m_entityIt = m_archetypeIt->second.begin();
        }
        return *this;
    }

    inline void operator++(int) { ++(*this); }
    inline bool operator==(std::default_sentinel_t) const { return m_archetypeIt == m_world->m_archetypes.end(); }
};

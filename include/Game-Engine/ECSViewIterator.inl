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
    template<typename C>
    using ComponentReference = std::conditional_t<std::is_const_v<ECSWorldT>, const C&, C&>;

    struct value_type
    {
        ECSWorldT* world = nullptr;
        typename ECSWorldT::EntityID entityId = INVALID_ENTITY_ID;
        std::tuple<ComponentReference<Cs>...> components;

        inline operator typename ECSWorldT::EntityID() const { return entityId; }
    };

private:
    friend class basic_ecsView;
    using ArchetypeMap = decltype(std::declval<ECSWorldT>().m_archetypes);

public:
    Iterator() = default;
    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) = default;

    ~Iterator() = default;

private:
    Iterator(ECSWorldT* world, const Predicate& predicate, ArchetypeMap::iterator archetypeIt, ECSWorldT::Archetype::Iterator entityIt)
        : m_world(world)
        , m_predicate(predicate)
        , m_archetypeIt(archetypeIt)
        , m_entityIt(entityIt)
    {
    }

    ECSWorldT* m_world = nullptr;
    Predicate m_predicate;
    ArchetypeMap::iterator m_archetypeIt;
    ECSWorldT::Archetype::Iterator m_entityIt;

public:
    Iterator& operator=(const Iterator& cp) = default;
    Iterator& operator=(Iterator&& mv) = default;

    inline value_type operator*() const
    {
        auto& archetype = m_archetypeIt->second;
        return value_type{
            .world = m_world,
            .entityId = *m_entityIt,
            .components = std::tuple<ComponentReference<Cs>...>(*m_archetypeIt->template getComponentPointer<Cs>(m_entityIt.idx())...)
        };
    }

    inline Iterator& operator++()
    {
        ++m_entityIt;
        if (m_entityIt == m_archetypeIt->second.end())
        {
            do ++m_archetypeIt;
            while(std::ranges::includes(m_archetypeIt->first, m_predicate) == false);
            if (m_archetypeIt != m_world->m_archetypes.end())
                m_entityIt = m_archetypeIt->second.begin();
        }
        return *this;
    }

    inline void operator++(int) { ++(*this); }
    inline bool operator==(std::default_sentinel_t) const { return m_archetypeIt == m_world->m_archetypes.end(); }
};

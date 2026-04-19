class Iterator
{
private:
    friend class ECSWorld;

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using iterator_concept = std::bidirectional_iterator_tag;
    using value_type = EntityID;
    using difference_type = std::ptrdiff_t;
    using reference = EntityID;

    Iterator() = default;
    Iterator(const Iterator& cp) = default;
    Iterator(Iterator&& mv) = default;

    ~Iterator() = default;

private:
    Iterator(const ECSWorld& world, EntityID id) : m_world(&world), m_id(id) {}

    const ECSWorld* m_world = nullptr;
    EntityID m_id = INVALID_ENTITY_ID;

public:
    Iterator& operator=(const Iterator& cp) = default;
    Iterator& operator=(Iterator&& mv) = default;

    inline EntityID operator*() const { return m_id; }

    inline Iterator& operator++()
    {
        do
        {
            ++m_id;
        } while (m_world->m_availableEntityIDs.contains(m_id));
        return *this;
    }
    inline Iterator operator++(int)
    {
        Iterator temp(*this);
        ++(*this);
        return temp;
    }

    inline Iterator& operator--()
    {
        do
        {
            --m_id;
        } while (m_world->m_availableEntityIDs.contains(m_id));
        return *this;
    }
    inline Iterator operator--(int)
    {
        Iterator temp(*this);
        --(*this);
        return temp;
    }

    inline bool operator==(const Iterator& rhs) const { return m_world == rhs.m_world && m_id == rhs.m_id; }
    inline bool operator!=(const Iterator& rhs) const { return !(*this == rhs); }
};

class ConstIterator
{
private:
    friend class ECSWorld;

public:
    using iterator_category = std::bidirectional_iterator_tag;
    using iterator_concept = std::bidirectional_iterator_tag;
    using value_type = EntityID;
    using difference_type = std::ptrdiff_t;
    using reference = EntityID;

    ConstIterator() = default;
    ConstIterator(const ConstIterator& cp) = default;
    ConstIterator(ConstIterator&& mv) = default;
    ConstIterator(const Iterator& it) : m_world(it.m_world), m_id(it.m_id) {}

    ~ConstIterator() = default;

private:
    ConstIterator(const ECSWorld& world, EntityID id) : m_world(&world), m_id(id) {}

    const ECSWorld* m_world = nullptr;
    EntityID m_id = INVALID_ENTITY_ID;

public:
    ConstIterator& operator=(const ConstIterator& cp) = default;
    ConstIterator& operator=(ConstIterator&& mv) = default;

    inline EntityID operator*() const { return m_id; }

    inline ConstIterator& operator++()
    {
        do
        {
            ++m_id;
        } while (m_world->m_availableEntityIDs.contains(m_id));
        return *this;
    }
    inline ConstIterator operator++(int)
    {
        ConstIterator temp(*this);
        ++(*this);
        return temp;
    }

    inline ConstIterator& operator--()
    {
        do
        {
            --m_id;
        } while (m_world->m_availableEntityIDs.contains(m_id));
        return *this;
    }
    inline ConstIterator operator--(int)
    {
        ConstIterator temp(*this);
        --(*this);
        return temp;
    }

    inline bool operator==(const ConstIterator& rhs) const { return m_world == rhs.m_world && m_id == rhs.m_id; }
    inline bool operator!=(const ConstIterator& rhs) const { return !(*this == rhs); }
};

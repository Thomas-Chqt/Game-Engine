class Archetype
{
public:
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;
        using value_type = EntityID;
        using difference_type = std::ptrdiff_t;

        Iterator() = default;
        Iterator(Archetype* archetype, uint64_t idx) : m_archetype(archetype), m_idx(idx) {}

        inline EntityID operator*() const { return m_archetype->getEntityID(m_idx); }
        inline Iterator& operator++() { ++m_idx; return *this; }
        inline Iterator operator++(int) { Iterator temp(*this); ++(*this); return temp; }
        inline bool operator==(const Iterator& rhs) const { return m_archetype == rhs.m_archetype && m_idx == rhs.m_idx; }
        inline bool operator!=(const Iterator& rhs) const { return !(*this == rhs); }
        inline uint64_t index() const { return m_idx; }

    private:
        Archetype* m_archetype = nullptr;
        uint64_t m_idx = 0;
    };

    class ConstIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using iterator_concept = std::forward_iterator_tag;
        using value_type = EntityID;
        using difference_type = std::ptrdiff_t;

        ConstIterator() = default;
        ConstIterator(const Archetype* archetype, uint64_t idx) : m_archetype(archetype), m_idx(idx) {}

        inline EntityID operator*() const { return m_archetype->getEntityID(m_idx); }
        inline ConstIterator& operator++() { ++m_idx; return *this; }
        inline ConstIterator operator++(int) { ConstIterator temp(*this); ++(*this); return temp; }
        inline bool operator==(const ConstIterator& rhs) const { return m_archetype == rhs.m_archetype && m_idx == rhs.m_idx; }
        inline bool operator!=(const ConstIterator& rhs) const { return !(*this == rhs); }
        inline uint64_t index() const { return m_idx; }

    private:
        const Archetype* m_archetype = nullptr;
        uint64_t m_idx = 0;
    };

    using iterator = Iterator;
    using const_iterator = ConstIterator;

    Archetype();
    Archetype(const Archetype&); // copy constructor make no sense inside the same ECSWorld. ment to be used only when copying the ECSWorld
    Archetype(Archetype&&);

    uint64_t size() const { return m_size; }

    inline iterator begin() { return iterator(this, 0); }
    inline iterator end() { return iterator(this, m_size); }
    inline const_iterator begin() const { return const_iterator(this, 0); }
    inline const_iterator end() const { return const_iterator(this, m_size); }
    inline const_iterator cbegin() const { return const_iterator(this, 0); }
    inline const_iterator cend() const { return const_iterator(this, m_size); }

    // must be use on empty achetype that are not inserted `ECSWorld::m_archetypes`
    template<typename T>
    void addRowType()
    {
        // clang-format off
        m_rows.insert(std::make_pair(componentID<T>(), Row{
            operator new (componentSize<T>() * m_capacity),
            componentSize<T>(),
            componentCopyConstructor<T>(),
            componentMoveConstructor<T>(),
            componentDestructor<T>(),
        }));
        // clang-format on
    }

    // must be use on empty achetype that are not inserted `ECSWorld::m_archetypes`
    template<typename T>
    void rmvRowType()
    {
        {
            Row& row = m_rows.at(componentID<T>());
            if (row.buffer != nullptr)
            {
                for (uint64_t i = 0; i < m_size; i++)
                    row.destructor(static_cast<std::byte*>(row.buffer) + (row.componentSize * i));
                operator delete(row.buffer);
            }
        }
        m_rows.erase(componentID<T>());
    }

    // only duplicate the component infos (no component duplicated)
    Archetype duplicateRowTypes();

    // only extend the size (and capacity if needed) and return last index
    uint64_t allocateCollum();

    template<Component T>
    T* getComponentPointer(uint64_t idx)
    {
        Row& row = m_rows.at(componentID<T>());
        return static_cast<T*>(row.buffer) + idx;
    }

    template<Component T>
    const T* getComponentPointer(uint64_t idx) const
    {
        const Row& row = m_rows.at(componentID<T>());
        return static_cast<T*>(row.buffer) + idx;
    }

    EntityID& getEntityID(uint64_t idx);
    const EntityID& getEntityID(uint64_t idx) const;

    // destination should be garbage memory
    // only call the move constructor
    static void moveComponents(Archetype& arcSrc, uint64_t idxSrc, Archetype& arcDst, uint64_t idxDst);

    // only call the destructor
    void destructCollum(uint64_t idx);

    // only reduce the size (and capacity if needed)
    void freeLastCollum();

    ~Archetype();

private:
    struct Row
    {
        void* buffer = nullptr;
        const uint64_t componentSize;
        const CopyConstructor copyConstructor;
        const MoveConstructor moveConstructor;
        const Destructor destructor;
    };

    std::map<ComponentID, Row> m_rows;
    uint64_t m_size;
    uint64_t m_capacity;

    void setCapacity(uint64_t);
    inline void extendCapacity() { setCapacity(m_capacity * 2); }
    inline void reduceCapacity() { setCapacity(m_capacity / 2 > 0 ? m_capacity / 2 : 1); }

public:
    Archetype& operator=(const Archetype&);
    Archetype& operator=(Archetype&&);
};

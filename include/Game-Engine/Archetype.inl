/*
 * ---------------------------------------------------
 * Archetype.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * ---------------------------------------------------
 */

class Archetype
{
public:
    class Iterator;
    using iterator = Iterator;

    Archetype();
    Archetype(const Archetype&); // copy constructor make no sense inside the same ECSWorld. ment to be used only when copying the ECSWorld
    Archetype(Archetype&&);

    uint64_t size() const { return m_size; }

    inline Iterator begin() const { return Iterator(this, 0); }
    inline std::default_sentinel_t end() const { return std::default_sentinel; }

    template<typename T> void addRowType(); // must be use on empty achetype that are not inserted `ECSWorld::m_archetypes`
    template<typename T> void rmvRowType(); // must be use on empty achetype that are not inserted `ECSWorld::m_archetypes`

    Archetype duplicateRowTypes(); // only duplicate the component infos (no component duplicated)
    uint64_t allocateCollum(); // only extend the size (and capacity if needed) and return last index

    template<Component T> auto* getComponentPointer(this auto&& self, uint64_t idx);
    auto getEntityID(this auto&& self, uint64_t idx)
        -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const EntityID&, EntityID&>;

    static void moveComponents(Archetype& arcSrc, uint64_t idxSrc, Archetype& arcDst, uint64_t idxDst); // only call the move constructor. destination should be garbage memory
    void destructCollum(uint64_t idx); // only call the destructor
    void freeLastCollum(); // only reduce the size (and capacity if needed)

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

public:
    #include "Game-Engine/ArchetypeIterator.inl"
};

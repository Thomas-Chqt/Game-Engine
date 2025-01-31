/*
 * ---------------------------------------------------
 * Archetype.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/16 16:36:28
 * ---------------------------------------------------
 */

#ifndef ECSWORLD_HPP
    #include "UtilsCPP/Dictionary.hpp"
    #include "UtilsCPP/Types.hpp"

    using EntityID = utils::uint64;
    using ComponentID = utils::uint32;
    using ArchetypeID = utils::Set<ComponentID>;

    using CopyConstructor = utils::Func<void(void* src, void* dst)>;
    using MoveConstructor = utils::Func<void(void* src, void* dst)>;
    using Destructor      = utils::Func<void(void* ptr)>;

    template<typename T> ComponentID componentID();
    template<typename T> utils::uint64 componentSize();
    template<typename T> const CopyConstructor& componentCopyConstructor();
    template<typename T> const MoveConstructor& componentMoveConstructor();
    template<typename T> const Destructor& componentDestructor();
#endif // ECSWORLD_HPP

class Archetype
{
public:
    Archetype();
    Archetype(const Archetype&); // copy constructor make no sense inside the same ECSWorld. ment to be used only when copying the ECSWorld
    Archetype(Archetype&&);

    utils::uint64 size() const { return m_size; }

    // must be use on empty achetype that are not inserted `ECSWorld::m_archetypes`
    template<typename T>
    void addRowType()
    {
        m_rows.insert(componentID<T>(), Row{
            operator new (componentSize<T>() * m_capacity),
            componentSize<T>(),
            componentCopyConstructor<T>(),
            componentMoveConstructor<T>(),
            componentDestructor<T>(),
        });
    }

    // must be use on empty achetype that are not inserted `ECSWorld::m_archetypes`
    template<typename T>
    void rmvRowType()
    {
        {
            Row& row = m_rows[componentID<T>()];
            if (row.buffer != nullptr)
            {
                for (utils::uint64 i = 0; i < m_size; i++)
                    row.destructor(static_cast<utils::byte*>(row.buffer) + (row.componentSize * i));
                operator delete (row.buffer);
            }            
        }
        m_rows.remove(componentID<T>());
    }

    // only duplicate the component infos (no component duplicated)
    Archetype duplicateRowTypes();

    // only extend the size (and capacity if needed) and return last index
    utils::uint64 allocateCollum();

    template<typename T>
    T* getComponentPointer(utils::uint64 idx)
    {
        Row& row = m_rows[componentID<T>()];
        return static_cast<T*>(row.buffer) + idx;
    }

    template<typename T>
    const T* getComponentPointer(utils::uint64 idx) const
    {
        const Row& row = m_rows[componentID<T>()];
        return static_cast<T*>(row.buffer) + idx;
    }
    
    EntityID& getEntityID(utils::uint64 idx);
    const EntityID& getEntityID(utils::uint64 idx) const;

    // destination should be garbage memory
    // only call the move constructor
    static void moveComponents(Archetype& arcSrc, utils::uint64 idxSrc, Archetype& arcDst, utils::uint64 idxDst);

    // only call the destructor
    void destructCollum(utils::uint64 idx);

    // only reduce the size (and capacity if needed)
    void freeLastCollum();

    ~Archetype();

private:
    struct Row
    {
        void* buffer = nullptr;
        const utils::uint64 componentSize;
        const CopyConstructor copyConstructor;
        const MoveConstructor moveConstructor;
        const Destructor destructor;
    };

    utils::Dictionary<ComponentID, Row> m_rows;
    utils::uint64 m_size;
    utils::uint64 m_capacity;

    void setCapacity(utils::uint64);
    inline void extendCapacity() { setCapacity(m_capacity * 2); }
    inline void reduceCapacity() { setCapacity(m_capacity / 2 > 0 ? m_capacity / 2 : 1); }

public:
    Archetype& operator = (const Archetype&);
    Archetype& operator = (Archetype&&);
};

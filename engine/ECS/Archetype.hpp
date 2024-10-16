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

    class ECSWorld;
    class CopyConstructor;
    class MoveConstructor;
    class Destructor;

    using ComponentID = utils::uint32;
    using ArchetypeID = utils::Set<ComponentID>;

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
    Archetype(const Archetype&);
    Archetype(Archetype&&);

    utils::uint64 size() const { return m_size; }

    template<typename T>
    void addRowType()
    {
        m_rows.insert(componentID<T>(), {
            operator new (m_capacity),
            componentSize<T>(),
            componentCopyConstructor<T>(),
            componentMoveConstructor<T>(),
            componentDestructor<T>(),
        });
    }

    template<typename T>
    void rmvRowType()
    {
        m_rows.remove(componentID<T>());
    }

    utils::uint64 allocateCollum();

    template<typename T>
    T* getComponentPointer(utils::uint64 idx)
    {
        Row& row = m_rows[componentID<T>()];
        return static_cast<T*>(row.buffer) + idx;
    }

    // destination should be garbage memory
    static void moveComponents(Archetype& arcSrc, utils::uint64 idxSrc, Archetype& arcDst, utils::uint64 idxDst);

    // only call the destructor
    void destructCollum(utils::uint64 idx);

    // only reduce the size (and capacity if needed)
    void freeLastCollum();

private:
    struct Row
    {
        void* buffer = nullptr;
        const utils::uint64 componentSize;
        const CopyConstructor& copyConstructor;
        const MoveConstructor& moveConstructor;
        const Destructor& destructor;
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

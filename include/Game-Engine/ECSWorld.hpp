/*
 * ---------------------------------------------------
 * ECSWorld.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/07/20 18:41:43
 * ---------------------------------------------------
 */

#ifndef ECSWORLD_HPP
#define ECSWORLD_HPP

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <set>
#include <functional>
#include <map>

#define INVALID_ENTITY_ID ULONG_MAX

namespace GE
{

class ECSWorld
{
public:
    using EntityID = uint64_t;

    class Iterator;

private:
    template<typename ... Ts> friend class ECSView;
    template<typename ... Ts> friend class const_ECSView;

    using ComponentID = uint32_t;
    using ArchetypeID = std::set<ComponentID>;

    using CopyConstructor = std::function<void(void* src, void* dst)>;
    using MoveConstructor = std::function<void(void* src, void* dst)>;
    using Destructor      = std::function<void(void* ptr)>;

public:
    ECSWorld();
    ECSWorld(const ECSWorld&) = default;
    ECSWorld(ECSWorld&&)      = default;

    EntityID newEntityID();

    void deleteEntityID(EntityID);

    inline bool isValidEntityID(EntityID id) const { return id < m_entityDatas.size() && m_availableEntityIDs.contains(id) == false; }

    template<typename T, typename ... Args>
    T& emplace(EntityID entityId, Args&& ... args);

    template<typename T>
    void remove(EntityID);

    template<typename T>
    bool has(EntityID) const;

    template<typename T>
    T& get(EntityID);

    template<typename T>
    const T& get(EntityID) const;

    inline uint32_t entityCount() { return m_entityDatas.size() - m_availableEntityIDs.size(); }
    inline uint32_t archetypeCount() { return m_archetypes.size(); }
    uint32_t componentCount();

    Iterator begin() const;
    Iterator end() const;

    ~ECSWorld() = default;

private:
    class Archetype
    {
    public:
        Archetype();
        Archetype(const Archetype&); // copy constructor make no sense inside the same ECSWorld. ment to be used only when copying the ECSWorld
        Archetype(Archetype&&);

        uint64_t size() const { return m_size; }

        // must be use on empty achetype that are not inserted `ECSWorld::m_archetypes`
        template<typename T>
        void addRowType()
        {
            m_rows.insert(std::make_pair(componentID<T>(), Row{
                operator new (componentSize<T>() * m_capacity),
                componentSize<T>(),
                componentCopyConstructor<T>(),
                componentMoveConstructor<T>(),
                componentDestructor<T>(),
            }));
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
                    operator delete (row.buffer);
                }
            }
            m_rows.erase(componentID<T>());
        }

        // only duplicate the component infos (no component duplicated)
        Archetype duplicateRowTypes();

        // only extend the size (and capacity if needed) and return last index
        uint64_t allocateCollum();

        template<typename T>
        T* getComponentPointer(uint64_t idx)
        {
            Row& row = m_rows.at(componentID<T>());
            return static_cast<T*>(row.buffer) + idx;
        }

        template<typename T>
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
        Archetype& operator = (const Archetype&);
        Archetype& operator = (Archetype&&);
    };

    struct EntityData
    {
        ArchetypeID archetypeId;
        uint64_t idx = 0;
    };

    static ComponentID nextComponentID();
    template<typename T> static ComponentID componentID();
    template<typename T> static uint64_t componentSize();
    template<typename T> static CopyConstructor componentCopyConstructor();
    template<typename T> static MoveConstructor componentMoveConstructor();
    template<typename T> static Destructor componentDestructor();

    std::vector<EntityData> m_entityDatas;
    std::set<EntityID> m_availableEntityIDs;

    std::map<ArchetypeID, Archetype> m_archetypes;

public:
    ECSWorld& operator = (const ECSWorld&) = default;
    ECSWorld& operator = (ECSWorld&&)      = default;

    // friend void to_json(nlohmann::json&, const ECSWorld&);
    // friend void from_json(const nlohmann::json&, ECSWorld&);

public:
    class Iterator
    {
    private:
        friend class ECSWorld;

    public:
        Iterator()                   = default;
        Iterator(const Iterator& cp) = default;
        Iterator(Iterator&& mv)      = default;

        ~Iterator() = default;

    private:
        Iterator(const ECSWorld& world, EntityID id) : m_world(&world), m_id(id) {}

        const ECSWorld* m_world = nullptr;
        EntityID m_id = INVALID_ENTITY_ID;

    public:
        Iterator& operator = (const Iterator& cp) = default;
        Iterator& operator = (Iterator&& mv)      = default;

        inline EntityID operator * () const { return m_id; }

        inline Iterator& operator ++ ()    { do { ++m_id; } while (m_world->m_availableEntityIDs.contains(m_id)); return *this; }
        inline Iterator  operator ++ (int) { Iterator temp(*this); ++(*this); return temp; }

        inline Iterator& operator -- ()    { do { --m_id; } while (m_world->m_availableEntityIDs.contains(m_id)); return *this; }
        inline Iterator  operator -- (int) { Iterator temp(*this); --(*this); return temp; }

        inline bool operator == (const Iterator& rhs) const { return m_world == rhs.m_world && m_id == rhs.m_id; }
        inline bool operator != (const Iterator& rhs) const { return !(*this == rhs); }
    };
};

template<typename T, typename ... Args>
T& ECSWorld::emplace(EntityID entityId, Args&& ... args)
{
    assert(isValidEntityID(entityId));
    assert(has<T>(entityId) == false);

    ArchetypeID& entityArchID = m_entityDatas[entityId].archetypeId;
    Archetype& entityArch = m_archetypes[entityArchID];
    uint64_t& entityIdx = m_entityDatas[entityId].idx;

    ArchetypeID dstArchID = entityArchID;
    dstArchID.insert(componentID<T>());
    auto it = m_archetypes.find(dstArchID);
    if (it == m_archetypes.end())
    {
        Archetype newArchetype = entityArch.duplicateRowTypes();
        newArchetype.addRowType<T>();
        auto [newIt, err] = m_archetypes.insert(std::make_pair(dstArchID, std::move(newArchetype)));
        assert(err);
        it = newIt;
    }
    Archetype& dstArchetype = it->second;
    uint64_t dstIdx = dstArchetype.allocateCollum();

    Archetype::moveComponents(entityArch, entityIdx, dstArchetype, dstIdx);

    m_entityDatas[entityArch.getEntityID(entityArch.size() - 1)].idx = entityIdx;

    entityArch.destructCollum(entityIdx);
    Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityIdx);
    entityArch.destructCollum(entityArch.size() - 1);
    entityArch.freeLastCollum();

    entityArchID = dstArchID;
    entityIdx = dstIdx;

    T* componentPtr = m_archetypes[entityArchID].getComponentPointer<T>(entityIdx);
    new (componentPtr) T(std::forward<Args>(args)...);
    return *componentPtr;
}

template<typename T>
void ECSWorld::remove(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    assert(has<T>(entityId));

    ArchetypeID& entityArchID = m_entityDatas[entityId].archetypeId;
    Archetype& entityArch = m_archetypes[entityArchID];
    uint64_t& entityIdx = m_entityDatas[entityId].idx;

    ArchetypeID dstArchID = entityArchID;
    dstArchID.erase(componentID<T>());
    auto it = m_archetypes.find(dstArchID);
    if (it == m_archetypes.end())
    {
        Archetype newArchetype = entityArch.duplicateRowTypes();
        newArchetype.rmvRowType<T>();
        auto [newIt, err] = m_archetypes.insert(std::make_pair(dstArchID, std::move(newArchetype)));
        assert(err);
        it = newIt;
    }
    Archetype& dstArchetype = it->second;
    uint64_t dstIdx = dstArchetype.allocateCollum();

    Archetype::moveComponents(entityArch, entityIdx, dstArchetype, dstIdx);

    m_entityDatas[entityArch.getEntityID(entityArch.size() - 1)].idx = entityIdx;

    if (entityIdx != entityArch.size() - 1)
    {
        entityArch.destructCollum(entityIdx);
        Archetype::moveComponents(entityArch, entityArch.size() - 1, entityArch, entityIdx);
    }
    entityArch.destructCollum(entityArch.size() - 1);
    entityArch.freeLastCollum();

    entityArchID = dstArchID;
    entityIdx = dstIdx;
}

template<typename T>
bool ECSWorld::has(EntityID entityId) const
{
    assert(isValidEntityID(entityId));
    return m_entityDatas[entityId].archetypeId.contains(componentID<T>());
}

template<typename T>
T& ECSWorld::get(EntityID entityId)
{
    assert(isValidEntityID(entityId));
    assert(has<T>(entityId));

    Archetype& entityArch = m_archetypes[m_entityDatas[entityId].archetypeId];
    uint64_t& entityIdx = m_entityDatas[entityId].idx;

    return *entityArch.getComponentPointer<T>(entityIdx);
}

template<typename T>
const T& ECSWorld::get(EntityID entityId) const
{
    assert(isValidEntityID(entityId));
    assert(has<T>(entityId));

    const Archetype& entityArch = m_archetypes.at(m_entityDatas[entityId].archetypeId);
    const uint64_t& entityIdx = m_entityDatas[entityId].idx;

    return *entityArch.getComponentPointer<T>(entityIdx);
}

template<typename T>
ECSWorld::ComponentID ECSWorld::componentID()
{
    static ComponentID id = nextComponentID();
    return id;
}

template<typename T>
uint64_t ECSWorld::componentSize()
{
    uint64_t size = sizeof(T);
    return size;
}

template<typename T>
ECSWorld::CopyConstructor ECSWorld::componentCopyConstructor()
{
    auto fn = [](void* src, void* dst) { new (dst) T(*(T*)src); };
    return fn;
}

template<typename T>
ECSWorld::MoveConstructor ECSWorld::componentMoveConstructor()
{
    auto fn = [](void* src, void* dst) { new (dst) T(std::move(*(T*)src)); };
    return fn;
}

template<typename T>
ECSWorld::Destructor ECSWorld::componentDestructor()
{
    auto fn = [](void* ptr) { ((T*)ptr)->~T(); };
    return fn;
}

}

#endif // ECSWORLD_HPP

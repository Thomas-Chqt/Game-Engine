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

#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <climits>

#define INVALID_ENTITY_ID ULONG_MAX

namespace GE
{

class ECSWorld
{
public:
    using EntityID = utils::uint64;
    using ComponentID = utils::uint32;

private:
    using ArchetypeID = utils::Set<ComponentID>;

private:
    template<typename ... Ts> friend class ECSView;

public:
    ECSWorld()                = default;
    ECSWorld(const ECSWorld&) = delete;
    ECSWorld(ECSWorld&&)      = default;


    EntityID newEntityID();

    void deleteEntityID(EntityID);
    inline bool isValidEntityID(EntityID id) const { return m_entityDatas.length() > id && m_availableEntityIDs.contain(id) == false; }

    template<typename T, typename ... Args>
    T& emplace(EntityID entityId, Args&& ... args);

    template<typename T>
    void remove(EntityID);

    template<typename T>
    bool has(EntityID);

    template<typename T>
    void* get(EntityID);

    inline utils::uint32 entityCount() { return m_entityDatas.length() - m_availableEntityIDs.size(); }
    inline utils::uint32 archetypeCount() { return m_archetypes.size(); }
    utils::uint32 componentCount();

    ~ECSWorld() = default;

private:
    struct Archetype
    {
        struct Row
        {
            void* buffer = nullptr;

            const utils::uint64 componentSize;
            const utils::Func<void(void* src, void* dst)>& componentCopyConstructor;
            const utils::Func<void(void* src, void* dst)>& componentMoveConstructor;
            const utils::Func<void(void* ptr)>& componentDestructor;

            Row(
                const utils::uint64 componentSize,
                const utils::Func<void(void* src, void* dst)>& componentCopyConstructor,
                const utils::Func<void(void* src, void* dst)>& componentMoveConstructor,
                const utils::Func<void(void* ptr)>& componentDestructor
            )
            : componentSize(componentSize),
              componentCopyConstructor(componentCopyConstructor),
              componentMoveConstructor(componentMoveConstructor),
              componentDestructor(componentDestructor) {}
        };

        const ArchetypeID id;
        utils::Dictionary<ECSWorld::ComponentID, Row> rows;
        utils::Array<utils::uint64> entityIdsRow;

        inline Archetype(const ArchetypeID& id) : id(id) {}
        
        inline bool operator  < (Archetype& rhs) const { return id  < rhs.id; }
        inline bool operator == (Archetype& rhs) const { return id == rhs.id; }

        inline bool operator  < (ArchetypeID& rhs) const { return id  < rhs; }
        inline bool operator == (ArchetypeID& rhs) const { return id == rhs; }
    };
    
    struct EntityData
    {
        Archetype* archetype = nullptr;
        utils::uint32 idx = 0;
    };

    inline static ComponentID nextComponentID() { static ComponentID id = 0; return id++; };
    template<typename T> inline static ComponentID componentID() { static ComponentID id = nextComponentID(); return id; }
    template<typename T> inline static utils::uint64 componentSize() { static utils::uint64 size = sizeof(T); return size; }
    template<typename T> inline static const utils::Func<void(void* src, void* dst)>& componentCopyConstructor() { static auto fn = [](void* src, void* dst) { new (dst) T(*(T*)src); }; return fn; }
    template<typename T> inline static const utils::Func<void(void* src, void* dst)>& componentMoveConstructor() { static auto fn = [](void* src, void* dst) { new (dst) T(std::move(*(T*)src)); }; return fn; }
    template<typename T> inline static const utils::Func<void(void* ptr)>& componentDestructor() { static auto fn = [](void* ptr) { ((T*)ptr)->~T(); }; return fn; }

    void moveComponents(Archetype& arcSrc, utils::uint64 idxSrc, Archetype& arcDst, utils::uint64 idxDst);
    void destructComponents(Archetype&, utils::uint64);
    void freeComponents(Archetype&, utils::uint64);
    void changeArchetypeRowCapacity(Archetype&, utils::uint64);
    
    Archetype& findOrCreateArchetype(const ArchetypeID&);

    utils::Array<EntityData> m_entityDatas;
    utils::Set<EntityID> m_availableEntityIDs;
    
    utils::Set<utils::UniquePtr<Archetype>> m_archetypes;
    
public:
    ECSWorld& operator = (const ECSWorld&) = delete;
    ECSWorld& operator = (ECSWorld&&)      = default;
};

template<typename T, typename ... Args>
T& ECSWorld::emplace(EntityID entityId, Args&& ... args)
{
    EntityData& entityData = m_entityDatas[entityId];
    ArchetypeID newArchetypeId = { componentID<T>() };
    if (entityData.archetype)
        newArchetypeId += entityData.archetype->id;
    Archetype& newArchetype = findOrCreateArchetype(newArchetypeId);
    utils::Array<utils::uint64>::Index newArchetypeAvailableIdx;
    // if (newArchetype.)
        
}

}

#endif // ECSWORLD_HPP
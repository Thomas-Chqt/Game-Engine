/*
 * ---------------------------------------------------
 * ECSWorld.hpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/07/20 18:41:43
 * ---------------------------------------------------
 */

#ifndef ECSWORLD_HPP
# define ECSWORLD_HPP

#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

using EntityID    = utils::uint64;
using ComponentID = utils::uint32;

class Archetype;

class ECSWorld
{
public:
    ECSWorld()                = default;
    ECSWorld(const ECSWorld&) = delete;
    ECSWorld(ECSWorld&&)      = delete;
    
    EntityID createEntity();
    void deleteEntity(EntityID);
    
    template<typename T> inline void addComponent(EntityID entity, const T& component) { addComponent(entity, componentID<T>(), sizeof(T), [](void* ptr){ ((T*)ptr)->~T(); }); }
    template<typename T> void removeComponent(EntityID);

    ~ECSWorld() = default;

private:
    struct EntityData
    {
        Archetype* archetype = nullptr;
        utils::uint32 idx = 0;
    };

    template<typename T>
    ComponentID componentID()
    {
        static ComponentID id = m_nextComponentID++;
        return id;
    }

    void addComponent(EntityID, ComponentID, void* data, utils::uint32 size, utils::Func<void(void*)> destructor);
    void removeComponent(EntityID, ComponentID);

    Archetype* newArchetype();

    ComponentID m_nextComponentID = 0;

    utils::Array<EntityData> m_entityDatas;
    utils::Array<EntityID> m_availableEntityIDs;
    
    utils::Dictionary<ComponentID, utils::UniquePtr<Archetype>> m_rootArchetypes;
    utils::Array<utils::UniquePtr<Archetype>> m_archetypes;
    
public:
    ECSWorld& operator = (const ECSWorld&) = delete;
    ECSWorld& operator = (ECSWorld&&)      = delete;
};

}

#endif // ECSWORLD_HPP
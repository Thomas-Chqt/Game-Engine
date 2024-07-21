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

#include "Archetype.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class ECSWorld;

struct Entity
{
    ECSWorld* ecsWorld;
    utils::uint64 idx;

    template<typename T> inline T& get() const;
    template<typename T> inline void add(const T& component);
    template<typename T> inline void remove();
};

class ECSWorld
{
public:
    ECSWorld()                = default;
    ECSWorld(const ECSWorld&) = delete;
    ECSWorld(ECSWorld&&)      = delete;
    
    Entity createEntity();
    void deleteEntity(Entity entt);
    
    template<typename T> T& getComponent(utils::uint64 idx) const;
    template<typename T> void addComponent(utils::uint64 idx, const T& component);
    template<typename T> void removeComponent(utils::uint64 idx);

    ~ECSWorld() = default;

private:
    struct EntityData
    {
        Archetype::ID archetypeID;
        Archetype::Index idx;
    };

    utils::Array<EntityData> m_entityDatas;
    utils::Array<utils::uint64> m_availableIndices;
    
    utils::Dictionary<Archetype::ID, utils::UniquePtr<Archetype>> m_archetypes;
    
public:
    ECSWorld& operator = (const ECSWorld&) = delete;
    ECSWorld& operator = (ECSWorld&&)      = delete;
};

template<typename T> T& Entity::get() const { return ecsWorld->getComponent<T>(idx); }
template<typename T> void Entity::add(const T& component) { return ecsWorld->addComponent<T>(idx, component); }
template<typename T> void Entity::remove() { return ecsWorld->removeComponent<T>(idx); }

}

#endif // ECSWORLD_HPP
/*
 * ---------------------------------------------------
 * Entity.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/02 14:20:00
 * ---------------------------------------------------
 */

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "ECSWorld.hpp"

namespace GE
{

class Entity
{
public:
    Entity()              = default;
    Entity(const Entity&) = default;
    Entity(Entity&&)      = default;

    inline Entity(ECSWorld& world) : m_world(&world), m_id(world.createEntity()) {}
    inline Entity(ECSWorld& world, ECSWorld::EntityID id) : m_world(&world), m_id(id) {}

    template<typename T> inline void add(const T& component) { m_world->addComponent(m_id, component); }
    template<typename T> inline void remove() { m_world->removeComponent<T>(m_id); }
    template<typename T> inline T& get() { return m_world->getComponent<T>(m_id); }
    template<typename ... Ts> inline bool has() { return m_world->hasComponents<Ts...>(m_id); }

    Entity& operator = (const Entity&) = default;
    Entity& operator = (Entity&&)      = default;

    bool operator == (const Entity& rhs) { return m_world == rhs.m_world && m_id == rhs.m_id; }    

private:
    ECSWorld* m_world = nullptr;
    ECSWorld::EntityID m_id = 0;
};

}

#endif // ENTITY_HPP
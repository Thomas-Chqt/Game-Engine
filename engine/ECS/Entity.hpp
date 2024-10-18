/*
 * ---------------------------------------------------
 * Entity.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 14:10:20
 * ---------------------------------------------------
 */

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "ECS/ECSWorld.hpp"

namespace GE
{

class Entity
{
public:
    Entity()              = default;
    Entity(const Entity&) = default;
    Entity(Entity&&)      = default;
    
    Entity(ECSWorld&, ECSWorld::EntityID);

    inline const ECSWorld& ecsWorld() const { return *m_world; }
    inline ECSWorld::EntityID entityID() const { return m_entityId; }
    inline void* imGuiID() const { return (void*)entityID(); }

    template<typename T, typename ... Args>
    T& emplace(Args&& ... args)
    {
        return m_world->emplace<T>(m_entityId, args...);
    }

    template<typename T>
    void remove()
    {
        m_world->remove<T>(m_entityId);
    }

    template<typename T>
    bool has()
    {
        return m_world->has<T>(m_entityId);
    }

    template<typename T>
    bool has() const
    {
        return m_world->has<T>(m_entityId);
    }

    template<typename T>
    T& get()
    {
        return m_world->get<T>(m_entityId);
    }

    template<typename T>
    const T& get() const
    {
        return m_world->get<T>(m_entityId);
    }
    
    void destroy()
    {
        m_world->deleteEntityID(m_entityId);
        m_world = nullptr;
        m_entityId = INVALID_ENTITY_ID;
    }

    ~Entity() = default;

private:
    ECSWorld* m_world = nullptr;
    ECSWorld::EntityID m_entityId = INVALID_ENTITY_ID;

public:
    Entity& operator = (const Entity&) = default;
    Entity& operator = (Entity&&)      = default;

    inline operator bool () { return m_world != nullptr && m_world->isValidEntityID(m_entityId); }
    inline bool operator == (const Entity& rhs) const { return m_world == rhs.m_world && m_entityId == rhs.m_entityId; }
    inline bool operator != (const Entity& rhs) const { return !(*this == rhs); }
};

}

#endif // ENTITY_HPP
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
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include "Macros.hpp"

namespace GE
{

class GAME_ENGINE_API Entity
{
public:
    Entity()              = default;
    Entity(const Entity&) = default;
    Entity(Entity&&)      = default;
    
    Entity(ECSWorld&, ECSWorld::EntityID);

    inline const ECSWorld& ecsWorld() const { return *m_world; }
    inline ECSWorld::EntityID entityID() const { return m_entityId; }
    inline void* imGuiID() const { return (void*)(uintptr_t)entityID(); }

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
    
    void destroy();

    utils::String& name();

    Entity parent();
    inline const Entity parent() const { return const_cast<Entity*>(this)->parent(); }

    Entity firstChild();
    inline const Entity firstChild() const { return const_cast<Entity*>(this)->firstChild(); }

    Entity nextChild();
    inline const Entity nextChild() const { return const_cast<Entity*>(this)->nextChild(); }

    bool hasParent() const;
    utils::uint32 childCount() const;
    Entity lastChild();
    const Entity lastChild() const;
    bool isParentOf(const Entity&) const; // or grand parent ect

    void addChild(Entity);
    void addChild(Entity child, Entity after);
    void removeChild(Entity);
    void removeParent();

    math::vec3f& position();
    const math::vec3f& position() const { return const_cast<Entity*>(this)->position(); }

    math::vec3f& rotation();
    const math::vec3f& rotation() const { return const_cast<Entity*>(this)->rotation(); }

    math::vec3f& scale();
    const math::vec3f& scale() const { return const_cast<Entity*>(this)->scale(); }

    math::mat4x4 transform() const;
    math::mat4x4 transform_noScale() const;

    math::mat4x4 worldTransform() const;
    math::mat4x4 worldTransform_noScale() const;

    ~Entity() = default;

private:
    ECSWorld* m_world = nullptr;
    ECSWorld::EntityID m_entityId = INVALID_ENTITY_ID;

public:
    Entity& operator = (const Entity&) = default;
    Entity& operator = (Entity&&)      = default;

    inline operator bool () const { return m_world != nullptr && m_world->isValidEntityID(m_entityId); }
    inline bool operator == (const Entity& rhs) const { return m_world == rhs.m_world && m_entityId == rhs.m_entityId; }
    inline bool operator != (const Entity& rhs) const { return !(*this == rhs); }
};

}

#endif // ENTITY_HPP
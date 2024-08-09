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

#include "ECSWorld.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/String.hpp"
#include <utility>

namespace GE
{

class Entity
{
public:
    Entity()              = default;
    Entity(const Entity&) = default;
    Entity(Entity&&)      = default;
    
    Entity(ECSWorld&, ECSWorld::EntityID);

    template<typename T, typename ... Args>
    inline T& emplace(Args&& ... args) { return m_world->emplace<T, Args...>(m_entityId, std::forward<Args>(args)...); }

    template<typename T>
    inline void remove() { m_world->remove<T>(m_entityId); }

    template<typename T>
    inline bool has() { return m_world->has<T>(m_entityId); }

    template<typename T>
    inline T& get() { return m_world->get<T>(m_entityId); }

    inline void destroy() { m_world->deleteEntity(m_entityId); }

    utils::String& name();
    math::mat4x4 transform();
    math::vec3f& position();
    math::vec3f& rotation();
    math::vec3f& scale();

    ~Entity() = default;

private:
    ECSWorld* m_world = nullptr;
    ECSWorld::EntityID m_entityId = INVALID_ENTITY_ID;

public:
    Entity& operator = (const Entity&) = default;
    Entity& operator = (Entity&&)      = default;

    inline operator bool () { return m_world != nullptr && m_world->isValid(m_entityId); }
};

class ScriptableEntity : public Entity
{
public:
    ScriptableEntity(Entity entity) : Entity(entity) {}

    virtual void onUpdate() = 0;

    virtual ~ScriptableEntity() = default;
};

}

#endif // ENTITY_HPP
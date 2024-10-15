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
#include "UtilsCPP/UniquePtr.hpp"
#include <type_traits>
#include <utility>
#include <type_traits>

namespace GE
{

class Entity
{
public:
    Entity()              = default;
    Entity(const Entity&) = default;
    Entity(Entity&&)      = default;
    
    Entity(ECSWorld&, ECSWorld::EntityID);

    inline void* imGuiID() { return (void*)m_entityId; }

    template<typename T, typename ... Args>
    T& emplace(Args&& ... args)
    {
        utils::uint32 size = sizeof(T);
        auto constructor     = [&](void* ptr)           { new (ptr) T{std::forward<Args>(args)...}; };
        auto copyConstructor = [](void* src, void* dst) { new (dst) T(*(T*)src); };
        auto moveConstructor = [](void* src, void* dst) { new (dst) T(std::move(*(T*)src)); };
        auto destructor      = [](void* ptr)            { ((T*)ptr)->~T(); };
        return *(T*)m_world->emplace(m_entityId, ECSWorld::componentID<T>(), size, constructor, copyConstructor, moveConstructor, destructor);
    }

    template<typename T> inline void remove() { m_world->remove(m_entityId, ECSWorld::componentID<T>()); }
    template<typename T> inline bool has() { return m_world->has(m_entityId, ECSWorld::componentID<T>()); }
    template<typename T> inline T& get() { return *(T*)m_world->get(m_entityId, ECSWorld::componentID<T>()); }
    
    void destroy()
    {
        m_world->deleteEntityID(m_entityId);
        m_world = nullptr;
        m_entityId = INVALID_ENTITY_ID;
    }

    utils::String& name();

    math::mat4x4 transform();
    math::vec3f& position();
    math::vec3f& rotation();
    math::vec3f& scale();
    
    Entity parent();
    Entity firstChild();
    Entity nextChild();
    
    template<typename T>
    void setScriptInstance()
    {
        static_assert(std::is_base_of<Entity, T>::value);
        utils::UniquePtr<T> instance = utils::makeUnique<T>(*m_world, m_entityId);
        setScriptInstance(instance.template staticCast<Entity>());
    }

    void pushChild(Entity);
    Entity popChild();
    math::mat4x4 worldTransform();

    virtual void onSetup() {}
    virtual void onUpdate() {}

    ~Entity() = default;

private:
    ECSWorld* m_world = nullptr;
    ECSWorld::EntityID m_entityId = INVALID_ENTITY_ID;

    void setScriptInstance(utils::UniquePtr<Entity>&&);

public:
    Entity& operator = (const Entity&) = default;
    Entity& operator = (Entity&&)      = default;

    inline operator bool () { return m_world != nullptr && m_world->isValidEntityID(m_entityId); }
    inline bool operator == (const Entity& rhs) const { return m_world == rhs.m_world && m_entityId == rhs.m_entityId; }
    inline bool operator != (const Entity& rhs) const { return !(*this == rhs); }
};

}

#endif // ENTITY_HPP
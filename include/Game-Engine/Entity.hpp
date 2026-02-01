/*
 * ---------------------------------------------------
 * Entity.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 14:10:20
 * ---------------------------------------------------
 *
 * Entity is just a handle that provide easier access to the component of an entity ID.
 * entity object can be pass by value, there is no raii so when an entity goes out of scop
 * nothing is deleted
 *
 */

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Components.hpp"

#include <cassert>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace GE
{

template<typename T>
concept ECSWorldLike = std::is_same_v<std::remove_const_t<T>, GE::ECSWorld>;

template<ECSWorldLike ECSWorldT>
struct basic_entity;

template<typename T>
concept EntityLike = std::is_same_v<std::remove_reference_t<T>, basic_entity<ECSWorld>>;

template<typename T>
concept ConstEntityLike = std::is_convertible_v<std::remove_reference_t<T>, const basic_entity<ECSWorld>> || std::is_same_v<std::remove_cvref_t<T>, basic_entity<const ECSWorld>>;

template<ECSWorldLike ECSWorldT>
struct basic_entity
{
    ECSWorldT* world = nullptr;
    typename ECSWorldT::EntityID entityId = INVALID_ENTITY_ID;

    template<Component T, typename... Args>
    inline T& emplace(this EntityLike auto&& self, Args&&... arg)
    {
        return self.world->template emplace<T>(self.entityId, std::forward<Args>(arg)...);
    }

    template<Component T>
    inline void remove(this EntityLike auto&& self)
    {
        self.world->template remove<T>(self.entityId);
    }

    template<Component T>
    inline bool has(this ConstEntityLike auto&& self)
    {
        return self.world->template has<T>(self.entityId);
    }

    template<Component T>
    inline auto& get(this auto&& self)
    {
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
            return static_cast<const std::remove_pointer_t<decltype(self.world)>*>(self.world)->template get<T>(self.entityId);
        else
            return self.world->template get<T>(self.entityId);
    }

    void destroy(this EntityLike auto&& self)
    {
        if (auto parent = self.parent())
            parent->removeChild(self);
        for (auto& child : self.childs())
            self.removeChild(child);

        self.world->deleteEntityID(self.entityId);
        self = basic_entity<ECSWorldT>();
    }

    inline auto& name(this auto&& self)
    {
        return self.template get<NameComponent>().name;
    }

    std::optional<basic_entity<ECSWorldT>> parent(this auto&& self)
    {
        if (self.template has<HierarchyComponent>())
        {
            auto& component = self.template get<HierarchyComponent>();
            if (component.parent != INVALID_ENTITY_ID)
            {
                assert(self.world->isValidEntityID(component.parent));
                return basic_entity<ECSWorldT>{
                    .world = self.world,
                    .entityId = component.parent
                };
            }
        }
        return std::nullopt;
    }

    std::optional<basic_entity<ECSWorldT>> firstChild(this auto&& self)
    {
        if (self.template has<HierarchyComponent>())
        {
            auto& component = self.template get<HierarchyComponent>();
            if (component.firstChildId != INVALID_ENTITY_ID)
            {
                assert(self.world->isValidEntityID(component.firstChildId));
                return basic_entity<ECSWorldT>{
                    .world = self.world,
                    .entityId = component.firstChildId
                };
            }
        }
        return std::nullopt;
    }

    std::optional<basic_entity<ECSWorldT>> nextChild(this auto&& self)
    {
        if (self.template has<HierarchyComponent>())
        {
            auto& component = self.template get<HierarchyComponent>();
            if (component.nextChild != INVALID_ENTITY_ID)
            {
                assert(self.world->isValidEntityID(component.nextChild));
                return basic_entity<ECSWorldT>{
                    .world = self.world,
                    .entityId = component.nextChild
                };
            }
        }
        return std::nullopt;
    }

    std::vector<basic_entity<ECSWorldT>> childs(this auto&& self)
    {
        std::vector<basic_entity<ECSWorldT>> vec;
        std::optional<basic_entity<ECSWorldT>> curr = self.firstChild();
        while (curr.has_value())
        {
            vec.push_back(*curr);
            curr = curr->nextChild();
        }
        return vec;
    }

    bool isParentOf(this ConstEntityLike auto&& self, ConstEntityLike auto& entity) // or grand parent ect
    {
        std::optional<basic_entity<ECSWorldT>> curr = entity.parent();
        while (curr)
        {
            if (curr == self)
                return true;
            curr = curr->parent();
        }
        return false;
    }

    template<ConstEntityLike T>
    void addChild(this EntityLike auto&& self, EntityLike auto& child, const std::optional<T>& after = std::nullopt)
    {
        assert(self.world == child.world);
        assert(self.isParentOf(child) == false);
        assert(child.parent() == std::nullopt);
        assert(!after || self.childs().contains(*after));

        HierarchyComponent& selfComp = self.template has<HierarchyComponent>() ? self.template get<HierarchyComponent>() : self.template emplace<HierarchyComponent>();
        HierarchyComponent& childComp = child.template has<HierarchyComponent>() ? child.template get<HierarchyComponent>() : child.template emplace<HierarchyComponent>();

        if (after.has_value())
        {
            HierarchyComponent& afterComp = after->template get<HierarchyComponent>();
            std::optional<basic_entity<ECSWorldT>> afterNext = after->nextChild();
            childComp.nextChild = afterNext ? afterNext->entityId : INVALID_ENTITY_ID;
            afterComp.nextChild = child.entityId;
        }
        else if (self.firstChild())
            return self.addChild(self.childs().back());
        else
            selfComp.firstChild = child.entityId;

        childComp.parent = self.entityId;
    }

    void removeChild(this EntityLike auto&& self, EntityLike auto& child)
    {
        assert(self.world == child.world);
        assert(self.childs().contains(child));

        HierarchyComponent& selfComp = self.template get<HierarchyComponent>();
        HierarchyComponent& childComp = child.template get<HierarchyComponent>();

        assert(self.firstChild().has_value());
        if (*self.firstChild() == child)
        {
            std::optional<basic_entity<ECSWorldT>> next = self.firstChild()->nextChild();
            selfComp.firstChild = next ? next->entityId : INVALID_ENTITY_ID;
        }
        else
        {
            basic_entity<ECSWorldT> curr = *self.firstChild();

            assert(curr.nextChild().has_value());
            while (*curr.nextChild() != child)
                curr = curr.nextChild();

            assert(curr.nextChild().has_value());
            curr.get<HierarchyComponent>().nextChild = curr.nextChild()->nextChild() ? curr.nextChild()->nextChild()->entityId : INVALID_ENTITY_ID;
        }

        childComp.parent = INVALID_ENTITY_ID;
        childComp.nextChild = INVALID_ENTITY_ID;
    }

    inline bool operator==(this ConstEntityLike auto&& lhs, ConstEntityLike auto& rhs) { return lhs.world == rhs.world && lhs.entityId == rhs.entityId; }
    inline bool operator!=(this ConstEntityLike auto&& lhs, ConstEntityLike auto& rhs) { return !(lhs == rhs); }

    inline operator basic_entity<const ECSWorld>() const requires std::is_same_v<ECSWorldT, ECSWorld> { return basic_entity<const ECSWorld>{ world, entityId }; }
};

using Entity = basic_entity<ECSWorld>;
using const_Entity = basic_entity<const ECSWorld>;

// class Entity
// {
// public:

//     bool hasParent() const;
//     utils::uint32 childCount() const;
//     Entity lastChild();
//     const Entity lastChild() const;

//     math::vec3f& position();
//     const math::vec3f& position() const { return const_cast<Entity*>(this)->position(); }

//     math::vec3f& rotation();
//     const math::vec3f& rotation() const { return const_cast<Entity*>(this)->rotation(); }

//     math::vec3f& scale();
//     const math::vec3f& scale() const { return const_cast<Entity*>(this)->scale(); }

//     math::mat4x4 transform() const;
//     math::mat4x4 transform_noScale() const;

//     math::mat4x4 worldTransform() const;
//     math::mat4x4 worldTransform_noScale() const;

//     ~Entity() = default;

// private:
//     ECSWorld* m_world = nullptr;
//     ECSWorld::EntityID m_entityId = INVALID_ENTITY_ID;

// public:
//     Entity& operator=(const Entity&) = default;
//     Entity& operator=(Entity&&) = default;

//     inline operator bool() const { return m_world != nullptr && m_world->isValidEntityID(m_entityId); }
// };

} // namespace GE

#endif // ENTITY_HPP

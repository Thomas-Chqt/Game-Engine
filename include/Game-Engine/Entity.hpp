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

#include <algorithm>
#include <cassert>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace GE
{

template<ECSWorldLike ECSWorldT>
struct basic_entity;

template<typename T>
concept EntityLike =
    std::is_same_v<std::remove_reference_t<T>, basic_entity<ECSWorld>>;

template<typename T>
concept ConstEntityLike =
    std::is_convertible_v<std::remove_reference_t<T>, const basic_entity<ECSWorld>> ||
    std::is_same_v<std::remove_cvref_t<T>, basic_entity<const ECSWorld>>;

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
        for (auto& child : self.children())
            self.removeChild(child);

        self.world->deleteEntityID(self.entityId);
        self = basic_entity<ECSWorldT>();
    }

    auto& name(this auto&& self)
    {
        if constexpr (EntityLike<decltype(self)>)
        {
            if (self.template has<NameComponent>() == false)
                return self.template emplace<NameComponent>().name;
        }
        return self.template get<NameComponent>().name;
    }

    inline glm::mat4 transform() const { return static_cast<glm::mat4>(get<TransformComponent>()); }

    glm::vec3 worldPosition() const
    {
        const TransformComponent& transform = get<TransformComponent>();
        if (auto parent = this->parent())
        {
            assert(parent->template has<TransformComponent>());
            const glm::vec3 scaledLocalPosition = parent->worldScale() * transform.position;
            return parent->worldPosition() + parent->worldRotation() * scaledLocalPosition;
        }
        return transform.position;
    }

    glm::mat3 worldRotation() const
    {
        const TransformComponent& transform = get<TransformComponent>();

        glm::mat4 rotationMatrix = glm::mat4(1.0f);
        rotationMatrix = glm::rotate(rotationMatrix, transform.rotation.x, glm::vec3(1, 0, 0));
        rotationMatrix = glm::rotate(rotationMatrix, transform.rotation.y, glm::vec3(0, 1, 0));
        rotationMatrix = glm::rotate(rotationMatrix, transform.rotation.z, glm::vec3(0, 0, 1));
        const glm::mat3 localRotation = glm::mat3(rotationMatrix);

        if (auto parent = this->parent())
        {
            assert(parent->template has<TransformComponent>());
            return parent->worldRotation() * localRotation;
        }
        return localRotation;
    }

    glm::vec3 worldScale() const
    {
        const TransformComponent& transform = get<TransformComponent>();
        if (auto parent = this->parent())
        {
            assert(parent->template has<TransformComponent>());
            return parent->worldScale() * transform.scale;
        }
        return transform.scale;
    }

    glm::mat4 worldTransform() const
    {
        if (auto parent = this->parent())
        {
            assert(parent->template has<TransformComponent>());
            return parent->worldTransform() * transform();
        }
        return transform();
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
            if (component.firstChild != INVALID_ENTITY_ID)
            {
                assert(self.world->isValidEntityID(component.firstChild));
                return basic_entity<ECSWorldT>{
                    .world = self.world,
                    .entityId = component.firstChild
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

    std::vector<basic_entity<ECSWorldT>> children(this auto&& self)
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

    void addChild(this EntityLike auto&& self, EntityLike auto& child, EntityLike auto& after)
    {
        assert(self.world == child.world);
        assert(child.isParentOf(self) == false);
        assert(child.parent() == std::nullopt);
        assert(std::ranges::contains(self.children(), after)); // the `after` entity need to be a direct child
        assert(self.template has<HierarchyComponent>()); // if it has `after` as a direct child it should have the hierarchy component
        assert(after.template has<HierarchyComponent>()); // if `after` is a direct child it should have the hierarchy component

        if (child.template has<HierarchyComponent>() == false)
            child.template emplace<HierarchyComponent>();

        HierarchyComponent& childComp = child.template get<HierarchyComponent>();
        HierarchyComponent& afterComp = after.template get<HierarchyComponent>();

        if (after.nextChild()) {
            childComp.nextChild = after.nextChild()->entityId;
        }
        afterComp.nextChild = child.entityId;
        childComp.parent = self.entityId;
    }

    void addChild(this EntityLike auto&& self, EntityLike auto& child)
    {
        assert(self.world == child.world);
        assert(child.isParentOf(self) == false);
        assert(child.parent() == std::nullopt);

        if (self.template has<HierarchyComponent>() == false)
            self.template emplace<HierarchyComponent>();
        if (child.template has<HierarchyComponent>() == false)
            child.template emplace<HierarchyComponent>();

        HierarchyComponent& selfComp = self.template get<HierarchyComponent>(); // need to be done after any emplace as `emplace` can invalidate references
        HierarchyComponent& childComp = child.template get<HierarchyComponent>();

        if (self.firstChild())
            return self.addChild(child, self.children().back());
        selfComp.firstChild = child.entityId;
        childComp.parent = self.entityId;
    }

    void removeChild(this EntityLike auto&& self, EntityLike auto& child)
    {
        assert(self.world == child.world);
        assert(std::ranges::contains(self.children(), child));

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
            while (*curr.nextChild() != child) { // we checked that child is a child of self so this will happen before `nextChild.has_value == false`
                assert(curr.nextChild().has_value());
                curr = *curr.nextChild();
            }

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

} // namespace GE

#endif // ENTITY_HPP

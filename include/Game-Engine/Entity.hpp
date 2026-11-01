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

#include "Game-Engine/ECSRegistry.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSView.hpp"

#include <tracy/Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace GE
{

template<ECSRegistryLike ECSRegistryT>
struct basic_entity;

template<typename T>
concept EntityLike =
    std::is_same_v<std::remove_reference_t<T>, basic_entity<ECSRegistry>>;

template<typename T>
concept ConstEntityLike =
    std::is_convertible_v<std::remove_reference_t<T>, const basic_entity<ECSRegistry>> ||
    std::is_same_v<std::remove_cvref_t<T>, basic_entity<const ECSRegistry>>;

template<ECSRegistryLike ECSRegistryT>
struct basic_entity
{
    ECSRegistryT* registry;
    EntityID entityId;

    basic_entity(ECSRegistryT* registry, EntityID id)
        : registry(registry), entityId(id)
    {
        assert(registry);
        assert(registry->isValidEntityID(entityId));
    }

    template<Component T, typename... Args>
    inline T& emplace(this EntityLike auto&& self, Args&&... arg)
    {
        return self.registry->template emplace<T>(self.entityId, std::forward<Args>(arg)...);
    }

    template<Component T>
    inline void remove(this EntityLike auto&& self)
    {
        self.registry->template remove<T>(self.entityId);
    }

    template<Component T>
    inline bool has(this ConstEntityLike auto&& self)
    {
        return self.registry->template has<T>(self.entityId);
    }

    template<Component T>
    inline auto& get(this auto&& self)
    {
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>)
            return static_cast<const std::remove_pointer_t<decltype(self.registry)>*>(self.registry)->template get<T>(self.entityId);
        else
            return self.registry->template get<T>(self.entityId);
    }

    void destroy(this EntityLike auto&& self)
    {
        if (auto parent = self.parent())
            parent->removeChild(self);
        for (auto& child : self.children())
            self.removeChild(child);

        self.registry->deleteEntityID(self.entityId);
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

    glm::mat4 localTransform() const
    {
        assert(has<TransformComponent>());
        return get<TransformComponent>().localTransform();
    }

    const glm::mat4& worldTransform() const
    {
        assert(has<TransformComponent>());
        return get<TransformComponent>().worldTransform;
    }

    void updateTransformHierarchy(this EntityLike auto&& self, const glm::mat4& parentTransform)
    {
        assert(self.template has<TransformComponent>());
        TransformComponent& transformComponent = self.template get<TransformComponent>();

        transformComponent.worldTransform = parentTransform * transformComponent.localTransform();

        for (std::optional<Entity> child = self.firstChild(); child; child = child->nextChild())
            child->updateTransformHierarchy(transformComponent.worldTransform);
    }

    void updateTransformHierarchy(this EntityLike auto&& self)
    {
        self.updateTransformHierarchy(self.parent().transform([](GE::Entity parent) -> glm::mat4 {
            return parent.has<GE::TransformComponent>() ? parent.worldTransform() : glm::mat4(1.0f);
        }).value_or(glm::mat4(1.0f)));
    }

    std::optional<basic_entity<ECSRegistryT>> parent(this auto&& self)
    {
        ZoneScoped;
        if (self.template has<ParentComponent>())
        {
            auto& component = self.template get<ParentComponent>();
            if (component.parent.has_value())
            {
                assert(self.registry->isValidEntityID(*component.parent));
                return basic_entity<ECSRegistryT>{self.registry, *component.parent};
            }
        }
        return std::nullopt;
    }

    std::optional<basic_entity<ECSRegistryT>> firstChild(this auto&& self)
    {
        ZoneScoped;
        if (self.template has<ChildrenComponent>())
        {
            auto& component = self.template get<ChildrenComponent>();
            if (component.firstChild.has_value())
            {
                assert(self.registry->isValidEntityID(*component.firstChild));
                return basic_entity<ECSRegistryT>{self.registry, *component.firstChild};
            }
        }
        return std::nullopt;
    }

    std::optional<basic_entity<ECSRegistryT>> nextChild(this auto&& self)
    {
        ZoneScoped;
        if (self.template has<ParentComponent>())
        {
            auto& component = self.template get<ParentComponent>();
            if (component.nextChild.has_value())
            {
                assert(self.registry->isValidEntityID(*component.nextChild));
                return basic_entity<ECSRegistryT>{self.registry, *component.nextChild};
            }
        }
        return std::nullopt;
    }

    std::vector<basic_entity<ECSRegistryT>> children(this auto&& self)
    {
        ZoneScoped;
        std::vector<basic_entity<ECSRegistryT>> vec;
        std::optional<basic_entity<ECSRegistryT>> curr = self.firstChild();
        while (curr.has_value())
        {
            vec.push_back(*curr);
            curr = curr->nextChild();
        }
        return vec;
    }

    bool isParentOf(this ConstEntityLike auto&& self, ConstEntityLike auto& entity) // or grand parent ect
    {
        auto curr = entity.parent();
        while (curr)
        {
            if (curr == self)
                return true;
            curr = curr->parent();
        }
        return false;
    }

    bool hasChild() const
    {
        return firstChild() != std::nullopt;
    }

    bool hasParent() const
    {
        return parent() != std::nullopt;
    }

    void addChild(this EntityLike auto&& self, EntityLike auto& child, EntityLike auto& after)
    {
        assert(self.registry == child.registry);
        assert(child.isParentOf(self) == false);
        assert(child.parent() == std::nullopt);
        assert(std::ranges::contains(self.children(), after)); // the `after` entity need to be a direct child
        assert(self.template has<ChildrenComponent>()); // if it has `after` as a direct child it should have the children component
        assert(after.template has<ParentComponent>()); // if `after` is a direct child it should have the parent component

        if (child.template has<ParentComponent>() == false)
            child.template emplace<ParentComponent>();

        ParentComponent& childComp = child.template get<ParentComponent>();
        ParentComponent& afterComp = after.template get<ParentComponent>();

        childComp.nextChild = afterComp.nextChild;
        afterComp.nextChild = child.entityId;
        childComp.parent = self.entityId;
    }

    void addChild(this EntityLike auto&& self, EntityLike auto& child)
    {
        assert(self.registry == child.registry);
        assert(child.isParentOf(self) == false);
        assert(child.parent() == std::nullopt);

        if (self.firstChild())
        {
            auto children = self.children();
            return self.addChild(child, children.back());
        }

        if (self.template has<ChildrenComponent>() == false)
            self.template emplace<ChildrenComponent>();
        if (child.template has<ParentComponent>() == false)
            child.template emplace<ParentComponent>();

        ChildrenComponent& selfComp = self.template get<ChildrenComponent>(); // need to be done after any emplace as `emplace` can invalidate references
        ParentComponent& childComp = child.template get<ParentComponent>();

        selfComp.firstChild = child.entityId;
        childComp.parent = self.entityId;
    }

    void removeChild(this EntityLike auto&& self, EntityLike auto& child)
    {
        assert(self.registry == child.registry);
        assert(std::ranges::contains(self.children(), child));

        ChildrenComponent& selfComp = self.template get<ChildrenComponent>();
        ParentComponent& childComp = child.template get<ParentComponent>();

        assert(self.firstChild().has_value());
        if (*self.firstChild() == child)
        {
            selfComp.firstChild = childComp.nextChild;
        }
        else
        {
            basic_entity<ECSRegistryT> curr = *self.firstChild();

            assert(curr.nextChild().has_value());
            while (*curr.nextChild() != child) { // we checked that child is a child of self so this will happen before `nextChild.has_value == false`
                assert(curr.nextChild().has_value());
                curr = *curr.nextChild();
            }

            assert(curr.nextChild().has_value());
            curr.template get<ParentComponent>().nextChild = childComp.nextChild;
        }

        const bool noChildrenLeft = !self.template get<ChildrenComponent>().firstChild.has_value();
        child.template remove<ParentComponent>();
        if (noChildrenLeft)
            self.template remove<ChildrenComponent>();
    }

    inline bool operator==(this ConstEntityLike auto&& lhs, ConstEntityLike auto& rhs) { return lhs.registry == rhs.registry && lhs.entityId == rhs.entityId; }
    inline bool operator!=(this ConstEntityLike auto&& lhs, ConstEntityLike auto& rhs) { return !(lhs == rhs); }

    inline operator basic_entity<const ECSRegistry>() const requires std::is_same_v<ECSRegistryT, ECSRegistry> { return basic_entity<const ECSRegistry>{ registry, entityId }; }
};

using Entity = basic_entity<ECSRegistry>;
using const_Entity = basic_entity<const ECSRegistry>;

struct MakeEntity {};

template<typename... Terms>
auto operator|(const basic_ecsView<ECSRegistry, Terms...>& view, const MakeEntity)
{
    return view | std::views::transform([registry = view.registry()](const auto& e) { return Entity{registry, static_cast<GE::EntityID>(e)}; });
}

template<typename... Terms>
auto operator|(const basic_ecsView<const ECSRegistry, Terms...>& view, const MakeEntity)
{
    return view | std::views::transform([registry = view.registry()](const auto& e) { return const_Entity{registry, static_cast<GE::EntityID>(e)}; });
}

} // namespace GE

#endif // ENTITY_HPP

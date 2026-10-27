/*
 * ---------------------------------------------------
 * Entity_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2026/02/01 11:09:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/ECSRegistry.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Components.hpp"

#include <set>

namespace GE_tests
{

namespace
{

GE::Entity makeEntity(GE::ECSRegistry& registry)
{
    return GE::Entity{&registry, registry.newEntityID()};
}

}

TEST(EntityTest, staticAssert)
{
    static_assert(!std::is_default_constructible_v<GE::Entity>);
    static_assert(!std::is_default_constructible_v<GE::const_Entity>);
    static_assert(std::is_same_v<decltype(GE::Entity::entityId), GE::EntityID>);
    static_assert(GE::EntityLike<GE::Entity>, "Entity is not EntityLike");
    static_assert(!GE::EntityLike<const GE::Entity>, "const Entity is EntityLike");
    static_assert(GE::ConstEntityLike<GE::Entity>, "Entity is not ConstEntityLike");
    static_assert(GE::ConstEntityLike<const GE::Entity>, "const Entity is not ConstEntityLike");

    static_assert(!GE::EntityLike<GE::const_Entity>, "const_Entity is EntityLike");
    static_assert(!GE::EntityLike<const GE::const_Entity>, "const const_Entity is EntityLike");
    static_assert(GE::ConstEntityLike<GE::const_Entity>, "const_Entity is not ConstEntityLike");
    static_assert(GE::ConstEntityLike<const GE::const_Entity>, "const const_Entity is not ConstEntityLike");
}

TEST(EntityTest, emplaceAndGetCameraComponent)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity entity = makeEntity(ecsRegistry);

    entity.emplace<GE::CameraComponent>().fov = 90.0f;
    ASSERT_FLOAT_EQ(entity.get<GE::CameraComponent>().fov, 90.0f);
}

TEST(EntityTest, emplaceAndGetParentComponent)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity entity = makeEntity(ecsRegistry);

    const auto referencedId = ecsRegistry.newEntityID();
    entity.emplace<GE::ParentComponent>().parent = referencedId;
    ASSERT_EQ(entity.get<GE::ParentComponent>().parent, referencedId);
}

TEST(EntityTest, emplaceAndGetChildrenComponent)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity entity = makeEntity(ecsRegistry);

    const auto referencedId = ecsRegistry.newEntityID();
    entity.emplace<GE::ChildrenComponent>().firstChild = referencedId;
    ASSERT_EQ(entity.get<GE::ChildrenComponent>().firstChild, referencedId);
}

template<typename T>
class EntityEmplaceRemoveHasTest : public testing::Test {};
using EntityEmplaceRemoveHasTestedComponents = ::testing::Types<GE::TransformComponent, GE::ParentComponent, GE::ChildrenComponent>;
TYPED_TEST_SUITE(EntityEmplaceRemoveHasTest, EntityEmplaceRemoveHasTestedComponents);

TYPED_TEST(EntityEmplaceRemoveHasTest, _)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity entity = makeEntity(ecsRegistry);

    EXPECT_FALSE(entity.has<TypeParam>());

    entity.emplace<TypeParam>();
    EXPECT_TRUE(entity.has<TypeParam>());

    entity.remove<TypeParam>();
    EXPECT_FALSE(entity.has<TypeParam>());
}

TEST(EntityTest, nameAccess)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity entity = makeEntity(ecsRegistry);

    EXPECT_FALSE(entity.has<GE::NameComponent>());
    EXPECT_EQ(entity.name(), GE::NameComponent().name);
    EXPECT_TRUE(entity.has<GE::NameComponent>());
    entity.name() = "Enemy";
    EXPECT_EQ(entity.name(), "Enemy");
    EXPECT_EQ(entity.get<GE::NameComponent>().name, "Enemy");
}

TEST(EntityTest, parentChildQueries)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity root = makeEntity(ecsRegistry);
    GE::Entity child1 = makeEntity(ecsRegistry);
    GE::Entity child2 = makeEntity(ecsRegistry);

    root.addChild(child1);
    root.addChild(child2);

    EXPECT_FALSE(root.has<GE::ParentComponent>());
    EXPECT_TRUE(root.has<GE::ChildrenComponent>());
    EXPECT_TRUE(child1.has<GE::ParentComponent>());
    EXPECT_FALSE(child1.has<GE::ChildrenComponent>());
    EXPECT_TRUE(child2.has<GE::ParentComponent>());
    EXPECT_FALSE(child2.has<GE::ChildrenComponent>());

    ASSERT_TRUE(root.firstChild().has_value());
    EXPECT_EQ(*root.firstChild(), child1);

    ASSERT_TRUE(child1.nextChild().has_value());
    EXPECT_EQ(*child1.nextChild(), child2);
    EXPECT_FALSE(child2.nextChild().has_value());

    auto children = root.children();
    ASSERT_EQ(children.size(), 2u);
    EXPECT_EQ(children[0], child1);
    EXPECT_EQ(children[1], child2);
}

TEST(EntityTest, isParentOf)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity root = makeEntity(ecsRegistry);
    GE::Entity child = makeEntity(ecsRegistry);
    GE::Entity grandChild = makeEntity(ecsRegistry);

    root.addChild(child);
    child.addChild(grandChild);

    EXPECT_TRUE(root.isParentOf(child));
    EXPECT_TRUE(root.isParentOf(grandChild));
    EXPECT_TRUE(child.isParentOf(grandChild));
    EXPECT_FALSE(grandChild.isParentOf(root));
}

TEST(EntityTest, addChildWithAfter)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity root = makeEntity(ecsRegistry);
    GE::Entity child1 = makeEntity(ecsRegistry);
    GE::Entity child2 = makeEntity(ecsRegistry);
    GE::Entity child3 = makeEntity(ecsRegistry);

    root.addChild(child1);
    root.addChild(child2);

    root.addChild(child3, child1);

    auto children = root.children();
    ASSERT_EQ(children.size(), 3u);
    EXPECT_EQ(children[0], child1);
    EXPECT_EQ(children[1], child3);
    EXPECT_EQ(children[2], child2);
}

TEST(EntityTest, removeChild)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity root = makeEntity(ecsRegistry);
    GE::Entity child1 = makeEntity(ecsRegistry);
    GE::Entity child2 = makeEntity(ecsRegistry);
    GE::Entity child3 = makeEntity(ecsRegistry);

    root.addChild(child1);
    root.addChild(child2);
    root.addChild(child3);

    root.removeChild(child2);

    EXPECT_FALSE(child2.parent().has_value());
    EXPECT_FALSE(child2.nextChild().has_value());
    EXPECT_FALSE(child2.has<GE::ParentComponent>());

    auto children = root.children();
    ASSERT_EQ(children.size(), 2u);
    EXPECT_EQ(children[0], child1);
    EXPECT_EQ(children[1], child3);

    root.removeChild(child1);
    root.removeChild(child3);
    EXPECT_FALSE(root.has<GE::ChildrenComponent>());
}

TEST(EntityTest, destroyEntity)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity parent = makeEntity(ecsRegistry);
    GE::Entity child1 = makeEntity(ecsRegistry);
    GE::Entity child2 = makeEntity(ecsRegistry);

    parent.addChild(child1);
    parent.addChild(child2);

    GE::EntityID parentId = parent.entityId;
    parent.destroy();

    EXPECT_FALSE(ecsRegistry.isValidEntityID(parentId));
    EXPECT_EQ(parent.registry, &ecsRegistry);
    EXPECT_EQ(parent.entityId, parentId);

    EXPECT_FALSE(child1.parent().has_value());
    EXPECT_FALSE(child1.nextChild().has_value());
    EXPECT_FALSE(child1.has<GE::ParentComponent>());
    EXPECT_FALSE(child2.parent().has_value());
    EXPECT_FALSE(child2.nextChild().has_value());
    EXPECT_FALSE(child2.has<GE::ParentComponent>());
}

TEST(EntityTest, destroyChildUnlinksFromParent)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity parent = makeEntity(ecsRegistry);
    GE::Entity child = makeEntity(ecsRegistry);

    parent.addChild(child);

    GE::EntityID childId = child.entityId;
    child.destroy();

    EXPECT_FALSE(ecsRegistry.isValidEntityID(childId));
    EXPECT_FALSE(parent.firstChild().has_value());
    EXPECT_FALSE(parent.has<GE::ChildrenComponent>());
}

TEST(EntityTest, rootEntitiesCanBeQueriedByExcludingParentComponent)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity root = makeEntity(ecsRegistry);
    GE::Entity otherRoot = makeEntity(ecsRegistry);
    GE::Entity child = makeEntity(ecsRegistry);

    root.emplace<GE::NameComponent>("root");
    otherRoot.emplace<GE::NameComponent>("other root");
    child.emplace<GE::NameComponent>("child");

    root.addChild(child);

    std::set<GE::EntityID> roots;
    for (GE::Entity entity : ecsRegistry | GE::ECSView<GE::NameComponent>().without<GE::ParentComponent>() | GE::MakeEntity{})
        roots.insert(entity.entityId);

    EXPECT_TRUE(roots.contains(root.entityId));
    EXPECT_TRUE(roots.contains(otherRoot.entityId));
    EXPECT_FALSE(roots.contains(child.entityId));
}

TEST(EntityTest, equalityOperators)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity entity1 = makeEntity(ecsRegistry);
    GE::Entity entity2 = makeEntity(ecsRegistry);
    GE::Entity entity3 = entity1;

    EXPECT_TRUE(entity1 == entity3);
    EXPECT_FALSE(entity1 != entity3);
    EXPECT_TRUE(entity1 != entity2);
}

TEST(EntityTest, conversionToConstEntity)
{
    GE::ECSRegistry ecsRegistry;
    GE::Entity entity = makeEntity(ecsRegistry);

    GE::const_Entity constEntity = entity;
    EXPECT_EQ(constEntity.registry, entity.registry);
    EXPECT_EQ(constEntity.entityId, entity.entityId);

    const GE::const_Entity constEntity2 = entity;
    EXPECT_EQ(constEntity2.registry, entity.registry);
    EXPECT_EQ(constEntity2.entityId, entity.entityId);
}

}

/*
 * ---------------------------------------------------
 * Entity_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2026/02/01 11:09:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Components.hpp"

namespace GE_tests
{

namespace
{

GE::Entity makeEntity(GE::ECSWorld& world)
{
    return GE::Entity{
        .world = &world,
        .entityId = world.newEntityID()
    };
}

}

TEST(EntityTest, staticAssert)
{
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
    GE::ECSWorld ecsWorld;
    GE::Entity entity = makeEntity(ecsWorld);

    entity.emplace<GE::CameraComponent>().fov = 90.0f;
    ASSERT_FLOAT_EQ(entity.get<GE::CameraComponent>().fov, 90.0f);
}

TEST(EntityTest, emplaceAndGetHierarchyComponent)
{
    GE::ECSWorld ecsWorld;
    GE::Entity entity = makeEntity(ecsWorld);

    entity.emplace<GE::HierarchyComponent>().parent = 123;
    ASSERT_FLOAT_EQ(entity.get<GE::HierarchyComponent>().parent, 123);
}

template<typename T>
class EntityEmplaceRemoveHasTest : public testing::Test {};
using EntityEmplaceRemoveHasTestedComponents = ::testing::Types<GE::TransformComponent, GE::HierarchyComponent>;
TYPED_TEST_SUITE(EntityEmplaceRemoveHasTest, EntityEmplaceRemoveHasTestedComponents);

TYPED_TEST(EntityEmplaceRemoveHasTest, _)
{
    GE::ECSWorld ecsWorld;
    GE::Entity entity = makeEntity(ecsWorld);

    EXPECT_FALSE(entity.has<TypeParam>());

    entity.emplace<TypeParam>();
    EXPECT_TRUE(entity.has<TypeParam>());

    entity.remove<TypeParam>();
    EXPECT_FALSE(entity.has<TypeParam>());
}

TEST(EntityTest, nameAccess)
{
    GE::ECSWorld ecsWorld;
    GE::Entity entity = makeEntity(ecsWorld);

    EXPECT_FALSE(entity.has<GE::NameComponent>());
    EXPECT_EQ(entity.name(), GE::NameComponent().name);
    EXPECT_TRUE(entity.has<GE::NameComponent>());
    entity.name() = "Enemy";
    EXPECT_EQ(entity.name(), "Enemy");
    EXPECT_EQ(entity.get<GE::NameComponent>().name, "Enemy");
}

TEST(EntityTest, parentChildQueries)
{
    GE::ECSWorld ecsWorld;
    GE::Entity root = makeEntity(ecsWorld);
    GE::Entity child1 = makeEntity(ecsWorld);
    GE::Entity child2 = makeEntity(ecsWorld);

    root.addChild(child1);
    root.addChild(child2);

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
    GE::ECSWorld ecsWorld;
    GE::Entity root = makeEntity(ecsWorld);
    GE::Entity child = makeEntity(ecsWorld);
    GE::Entity grandChild = makeEntity(ecsWorld);

    root.addChild(child);
    child.addChild(grandChild);

    EXPECT_TRUE(root.isParentOf(child));
    EXPECT_TRUE(root.isParentOf(grandChild));
    EXPECT_TRUE(child.isParentOf(grandChild));
    EXPECT_FALSE(grandChild.isParentOf(root));
}

TEST(EntityTest, addChildWithAfter)
{
    GE::ECSWorld ecsWorld;
    GE::Entity root = makeEntity(ecsWorld);
    GE::Entity child1 = makeEntity(ecsWorld);
    GE::Entity child2 = makeEntity(ecsWorld);
    GE::Entity child3 = makeEntity(ecsWorld);

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
    GE::ECSWorld ecsWorld;
    GE::Entity root = makeEntity(ecsWorld);
    GE::Entity child1 = makeEntity(ecsWorld);
    GE::Entity child2 = makeEntity(ecsWorld);
    GE::Entity child3 = makeEntity(ecsWorld);

    root.addChild(child1);
    root.addChild(child2);
    root.addChild(child3);

    root.removeChild(child2);

    EXPECT_FALSE(child2.parent().has_value());
    EXPECT_FALSE(child2.nextChild().has_value());

    auto children = root.children();
    ASSERT_EQ(children.size(), 2u);
    EXPECT_EQ(children[0], child1);
    EXPECT_EQ(children[1], child3);
}

TEST(EntityTest, destroyEntity)
{
    GE::ECSWorld ecsWorld;
    GE::Entity parent = makeEntity(ecsWorld);
    GE::Entity child1 = makeEntity(ecsWorld);
    GE::Entity child2 = makeEntity(ecsWorld);

    parent.addChild(child1);
    parent.addChild(child2);

    GE::ECSWorld::EntityID parentId = parent.entityId;
    parent.destroy();

    EXPECT_FALSE(ecsWorld.isValidEntityID(parentId));
    EXPECT_EQ(parent.world, nullptr);
    EXPECT_EQ(parent.entityId, INVALID_ENTITY_ID);

    EXPECT_FALSE(child1.parent().has_value());
    EXPECT_FALSE(child1.nextChild().has_value());
    EXPECT_FALSE(child2.parent().has_value());
    EXPECT_FALSE(child2.nextChild().has_value());
}

TEST(EntityTest, destroyChildUnlinksFromParent)
{
    GE::ECSWorld ecsWorld;
    GE::Entity parent = makeEntity(ecsWorld);
    GE::Entity child = makeEntity(ecsWorld);

    parent.addChild(child);

    GE::ECSWorld::EntityID childId = child.entityId;
    child.destroy();

    EXPECT_FALSE(ecsWorld.isValidEntityID(childId));
    EXPECT_FALSE(parent.firstChild().has_value());
}

TEST(EntityTest, equalityOperators)
{
    GE::ECSWorld ecsWorld;
    GE::Entity entity1 = makeEntity(ecsWorld);
    GE::Entity entity2 = makeEntity(ecsWorld);
    GE::Entity entity3 = entity1;

    EXPECT_TRUE(entity1 == entity3);
    EXPECT_FALSE(entity1 != entity3);
    EXPECT_TRUE(entity1 != entity2);
}

TEST(EntityTest, conversionToConstEntity)
{
    GE::ECSWorld ecsWorld;
    GE::Entity entity = makeEntity(ecsWorld);

    GE::const_Entity constEntity = entity;
    EXPECT_EQ(constEntity.world, entity.world);
    EXPECT_EQ(constEntity.entityId, entity.entityId);

    const GE::const_Entity constEntity2 = entity;
    EXPECT_EQ(constEntity2.world, entity.world);
    EXPECT_EQ(constEntity2.entityId, entity.entityId);
}

}

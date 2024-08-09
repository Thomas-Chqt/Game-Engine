/*
 * ---------------------------------------------------
 * ECS_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 11:08:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/Entity.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Set.hpp"

#include "Game-Engine/ECSWorld.hpp"
#include "ECS/ECSView.hpp"

TEST(ECSTest, entities)
{
    GE::ECSWorld world;

    GE::ECSWorld::EntityID entity1 = world.newEntity();
    EXPECT_EQ(entity1, 0);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    GE::ECSWorld::EntityID entity2 = world.newEntity();
    EXPECT_EQ(entity2, 1);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);
    
    world.deleteEntity(entity1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    GE::ECSWorld::EntityID entity3 = world.newEntity();
    EXPECT_EQ(entity3, 0);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

}

TEST(ECSTest, oneComponent)
{
    struct Component1 { int value; };

    GE::ECSWorld world;

    GE::ECSWorld::EntityID entityId = world.newEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    EXPECT_EQ(world.emplace<Component1>(entityId, 1).value, 1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(world.get<Component1>(entityId).value, 1);
}

TEST(ECSTest, twoComponent)
{
    struct Component1 { int value; };
    struct Component2 { int value; };

    GE::ECSWorld world;

    GE::ECSWorld::EntityID entityId = world.newEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    EXPECT_EQ(world.emplace<Component1>(entityId, 1).value, 1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(world.emplace<Component2>(entityId, 2).value, 2);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_EQ(world.get<Component1>(entityId).value, 1);
    EXPECT_EQ(world.get<Component2>(entityId).value, 2);

    world.deleteEntity(entityId);
}

TEST(ECSTest, removeComponent)
{
    struct Component1 { utils::String value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::ECSWorld::EntityID entityId = world.newEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    world.emplace<Component1>(entityId, utils::String("1"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    world.emplace<Component2>(entityId, utils::String("2"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    world.remove<Component1>(entityId);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(world.get<Component2>(entityId).value, utils::String("2"));
    EXPECT_FALSE(world.has<Component1>(entityId));
}

TEST(ECSTest, removeComponent2)
{
    struct Component1 { utils::String value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::ECSWorld::EntityID entityId = world.newEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    world.emplace<Component1>(entityId, utils::String("1"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    world.emplace<Component2>(entityId, utils::String("2"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    world.remove<Component2>(entityId);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(world.get<Component1>(entityId).value, utils::String("1"));
    EXPECT_FALSE(world.has<Component2>(entityId));
}

TEST(ECSTest, multipleEntity)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::ECSWorld::EntityID entityId1 = world.newEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    world.emplace<Component1>(entityId1, 1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_TRUE(world.has<Component1>(entityId1));
    EXPECT_EQ(world.get<Component1>(entityId1).value, 1);

    world.emplace<Component2>(entityId1, utils::String("a"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_TRUE(world.has<Component2>(entityId1));
    EXPECT_EQ(world.get<Component2>(entityId1).value, utils::String("a"));

    world.remove<Component1>(entityId1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_FALSE(world.has<Component1>(entityId1));


    GE::ECSWorld::EntityID entityId2 = world.newEntity();
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    world.emplace<Component1>(entityId2, 2);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_TRUE(world.has<Component1>(entityId2));
    EXPECT_EQ(world.get<Component1>(entityId2).value, 2);

    world.emplace<Component2>(entityId2, utils::String("b"));
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_TRUE(world.has<Component2>(entityId2));
    EXPECT_EQ(world.get<Component2>(entityId2).value, utils::String("b"));

    world.remove<Component1>(entityId2);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_FALSE(world.has<Component1>(entityId2));


    GE::ECSWorld::EntityID entityId3 = world.newEntity();
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    world.emplace<Component1>(entityId3, 3);
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_TRUE(world.has<Component1>(entityId3));
    EXPECT_EQ(world.get<Component1>(entityId3).value, 3);

    world.emplace<Component2>(entityId3, utils::String("c"));
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 4);

    EXPECT_TRUE(world.has<Component2>(entityId3));
    EXPECT_EQ(world.get<Component2>(entityId3).value, utils::String("c"));

    world.remove<Component1>(entityId3);
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_FALSE(world.has<Component1>(entityId3));
}

TEST(ECSTest, view)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    {
        GE::ECSWorld::EntityID entityId = world.newEntity();
        world.emplace<Component1>(entityId, 1);
    }
    {
        GE::ECSWorld::EntityID entityId = world.newEntity();
        world.emplace<Component2>(entityId, utils::String("2"));
    }
    {
        GE::ECSWorld::EntityID entityId = world.newEntity();
        world.emplace<Component1>(entityId, 3);
        world.emplace<Component2>(entityId, utils::String("3"));
    }

    {
        GE::ECSView<Component1> view(world);
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            utils::Set<int> values;
            view.onEach([&](GE::Entity, Component1& comp1){ values.insert(comp1.value); });
        });
    }
    {
        GE::ECSView<Component2> view(world);
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            utils::Set<utils::String> values;
            view.onEach([&](GE::Entity, Component2& comp2){ values.insert(comp2.value); });
        });
    }
}

TEST(ECSTest, componentEdit)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    {
        GE::ECSWorld::EntityID entityId = world.newEntity();
        world.emplace<Component1>(entityId, 1);
    }
    {
        GE::ECSWorld::EntityID entityId = world.newEntity();
        world.emplace<Component2>(entityId, utils::String("2"));
    }
    {
        GE::ECSWorld::EntityID entityId = world.newEntity();
        world.emplace<Component1>(entityId, 1);
        world.emplace<Component2>(entityId, utils::String("2"));
    }

    {
        GE::ECSView<Component1> view(world);
        view.onEach([&](GE::Entity, Component1& comp1){ comp1.value += 1; });
    }
    {
        GE::ECSView<Component2> view(world);
        view.onEach([&](GE::Entity, Component2& comp2){ comp2.value[0] += 1; });
    }
    {
        GE::ECSView<Component1> view(world);
        view.onEach([&](GE::Entity, Component1& comp1){ EXPECT_EQ(comp1.value, 2); });
    }
    {
        GE::ECSView<Component2> view(world);
        view.onEach([&](GE::Entity, Component2& comp2){ EXPECT_EQ(comp2.value, utils::String("3")); });
    }
}
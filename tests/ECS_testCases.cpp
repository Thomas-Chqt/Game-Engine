/*
 * ---------------------------------------------------
 * ECS_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 11:08:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Set.hpp"

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/ECSWorldView.hpp"

TEST(ECSTest, entities)
{
    GE::ECSWorld world;

    GE::EntityID entity1 = world.createEntity();
    EXPECT_EQ(entity1, 0);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    GE::EntityID entity2 = world.createEntity();
    EXPECT_EQ(entity2, 1);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);
    
    world.deleteEntity(entity1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    GE::EntityID entity3 = world.createEntity();
    EXPECT_EQ(entity3, 0);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

}

TEST(ECSTest, oneComponent)
{
    struct Component1 { int value; };

    GE::ECSWorld world;

    GE::EntityID entityId = world.createEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    world.addComponent(entityId, Component1{1});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(world.getComponent<Component1>(entityId).value, 1);
}

TEST(ECSTest, twoComponent)
{
    struct Component1 { int value; };
    struct Component2 { int value; };

    GE::ECSWorld world;

    GE::EntityID entityId = world.createEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    world.addComponent(entityId, Component1{1});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    world.addComponent(entityId, Component2{2});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_EQ(world.getComponent<Component1>(entityId).value, 1);
    EXPECT_EQ(world.getComponent<Component2>(entityId).value, 2);
}

TEST(ECSTest, removeComponent)
{
    struct Component1 { utils::String value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::EntityID entityId = world.createEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    world.addComponent(entityId, Component1{"1"});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    world.addComponent(entityId, Component2{"2"});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    world.removeComponent<Component1>(entityId);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(world.getComponent<Component2>(entityId).value, utils::String("2"));
    EXPECT_FALSE(world.hasComponents<Component1>(entityId));
}

TEST(ECSTest, removeComponent2)
{
    struct Component1 { utils::String value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::EntityID entityId = world.createEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    world.addComponent(entityId, Component1{"1"});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    world.addComponent(entityId, Component2{"2"});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    world.removeComponent<Component2>(entityId);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(world.getComponent<Component1>(entityId).value, utils::String("1"));
    EXPECT_FALSE(world.hasComponents<Component2>(entityId));
}

TEST(ECSTest, multipleEntity)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::EntityID entityId1 = world.createEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    world.addComponent(entityId1, Component1{1});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_TRUE(world.hasComponents<Component1>(entityId1));
    EXPECT_EQ(world.getComponent<Component1>(entityId1).value, 1);

    world.addComponent(entityId1, Component2{"a"});
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_TRUE(world.hasComponents<Component2>(entityId1));
    EXPECT_EQ(world.getComponent<Component2>(entityId1).value, utils::String("a"));

    world.removeComponent<Component1>(entityId1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_FALSE(world.hasComponents<Component1>(entityId1));


    GE::EntityID entityId2 = world.createEntity();
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    world.addComponent(entityId2, Component1{2});
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_TRUE(world.hasComponents<Component1>(entityId2));
    EXPECT_EQ(world.getComponent<Component1>(entityId2).value, 2);

    world.addComponent(entityId2, Component2{"b"});
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_TRUE(world.hasComponents<Component2>(entityId2));
    EXPECT_EQ(world.getComponent<Component2>(entityId2).value, utils::String("b"));

    world.removeComponent<Component1>(entityId2);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_FALSE(world.hasComponents<Component1>(entityId2));


    GE::EntityID entityId3 = world.createEntity();
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    world.addComponent(entityId3, Component1{3});
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_TRUE(world.hasComponents<Component1>(entityId3));
    EXPECT_EQ(world.getComponent<Component1>(entityId3).value, 3);

    world.addComponent(entityId3, Component2{"c"});
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 4);

    EXPECT_TRUE(world.hasComponents<Component2>(entityId3));
    EXPECT_EQ(world.getComponent<Component2>(entityId3).value, utils::String("c"));

    world.removeComponent<Component1>(entityId3);
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_FALSE(world.hasComponents<Component1>(entityId3));
}

TEST(ECSTest, view)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    {
        GE::EntityID entityId = world.createEntity();
        world.addComponent(entityId, Component1{1});
    }
    {
        GE::EntityID entityId = world.createEntity();
        world.addComponent(entityId, Component2{"2"});
    }
    {
        GE::EntityID entityId = world.createEntity();
        world.addComponent(entityId, Component1{3});
        world.addComponent(entityId, Component2{"3"});
    }

    {
        GE::ECSWorldView<Component1> view(world);
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            utils::Set<int> values;
            view.foreach([&](Component1& comp1){ values.insert(comp1.value); });
        });
    }
    {
        GE::ECSWorldView<Component2> view(world);
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            utils::Set<utils::String> values;
            view.foreach([&](Component2& comp2){ values.insert(comp2.value); });
        });
    }
}

TEST(ECSTest, componentEdit)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    {
        GE::EntityID entityId = world.createEntity();
        world.addComponent(entityId, Component1{1});
    }
    {
        GE::EntityID entityId = world.createEntity();
        world.addComponent(entityId, Component2{"2"});
    }
    {
        GE::EntityID entityId = world.createEntity();
        world.addComponent(entityId, Component1{1});
        world.addComponent(entityId, Component2{"2"});
    }

    {
        GE::ECSWorldView<Component1> view(world);
        view.foreach([&](Component1& comp1){ comp1.value += 1; });
    }
    {
        GE::ECSWorldView<Component2> view(world);
        view.foreach([&](Component2& comp2){ comp2.value[0] += 1; });
    }
    {
        GE::ECSWorldView<Component1> view(world);
        view.foreach([&](Component1& comp1){ EXPECT_EQ(comp1.value, 2); });
    }
    {
        GE::ECSWorldView<Component2> view(world);
        view.foreach([&](Component2& comp2){ EXPECT_EQ(comp2.value, utils::String("3")); });
    }
}
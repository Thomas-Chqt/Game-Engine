/*
 * ---------------------------------------------------
 * ECS_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 11:08:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "ECS/Entity.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Set.hpp"

#include "ECS/ECSWorld.hpp"
#include "ECS/ECSView.hpp"

TEST(ECSTest, entities)
{
    GE::ECSWorld world;

    GE::Entity entity1 = world.newEmptyEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    GE::Entity entity2 = world.newEmptyEntity();
    EXPECT_NE(entity1, entity2);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);
    
    entity1.destroy();
    EXPECT_FALSE(entity1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    GE::Entity entity3 = world.newEmptyEntity();
    EXPECT_NE(entity2, entity3);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

}

TEST(ECSTest, oneComponent)
{
    struct Component1 { int value; };

    GE::ECSWorld world;

    GE::Entity entityId = world.newEmptyEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    EXPECT_EQ(entityId.emplace<Component1>(1).value, 1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(entityId.get<Component1>().value, 1);
}

TEST(ECSTest, twoComponent)
{
    struct Component1 { int value; };
    struct Component2 { int value; };

    GE::ECSWorld world;

    GE::Entity entity = world.newEmptyEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    EXPECT_EQ(entity.emplace<Component1>(1).value, 1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(entity.emplace<Component2>(2).value, 2);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_EQ(entity.get<Component1>().value, 1);
    EXPECT_EQ(entity.get<Component2>().value, 2);

    entity.destroy();
}

TEST(ECSTest, removeComponent)
{
    struct Component1 { utils::String value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::Entity entity = world.newEmptyEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    entity.emplace<Component1>(utils::String("1"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    entity.emplace<Component2>(utils::String("2"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    entity.remove<Component1>();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(entity.get<Component2>().value, utils::String("2"));
    EXPECT_FALSE(entity.has<Component1>());
}

TEST(ECSTest, removeComponent2)
{
    struct Component1 { utils::String value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::Entity entity = world.newEmptyEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    entity.emplace<Component1>(utils::String("1"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    entity.emplace<Component2>(utils::String("2"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    entity.remove<Component2>();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_EQ(entity.get<Component1>().value, utils::String("1"));
    EXPECT_FALSE(entity.has<Component2>());
}

TEST(ECSTest, multipleEntity)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    GE::Entity entity1 = world.newEmptyEntity();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 0);
    EXPECT_EQ(world.componentCount(), 0);

    entity1.emplace<Component1>(1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_TRUE(entity1.has<Component1>());
    EXPECT_EQ(entity1.get<Component1>().value, 1);

    entity1.emplace<Component2>(utils::String("a"));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_TRUE(entity1.has<Component2>());
    EXPECT_EQ(entity1.get<Component2>().value, utils::String("a"));

    entity1.remove<Component1>();
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_FALSE(entity1.has<Component1>());


    GE::Entity entity2 = world.newEmptyEntity();
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 1);

    entity2.emplace<Component1>(2);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_TRUE(entity2.has<Component1>());
    EXPECT_EQ(entity2.get<Component1>().value, 2);

    entity2.emplace<Component2>(utils::String("b"));
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_TRUE(entity2.has<Component2>());
    EXPECT_EQ(entity2.get<Component2>().value, utils::String("b"));

    entity2.remove<Component1>();
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_FALSE(entity2.has<Component1>());


    GE::Entity entity3 = world.newEmptyEntity();
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    entity3.emplace<Component1>(3);
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_TRUE(entity3.has<Component1>());
    EXPECT_EQ(entity3.get<Component1>().value, 3);

    entity3.emplace<Component2>(utils::String("c"));
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 4);

    EXPECT_TRUE(entity3.has<Component2>());
    EXPECT_EQ(entity3.get<Component2>().value, utils::String("c"));

    entity3.remove<Component1>();
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 3);

    EXPECT_FALSE(entity3.has<Component1>());
}

TEST(ECSTest, view)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    {
        GE::Entity entity = world.newEmptyEntity();
        entity.emplace<Component1>(1);
    }
    {
        GE::Entity entity = world.newEmptyEntity();
        entity.emplace<Component2>(utils::String("2"));
    }
    {
        GE::Entity entity = world.newEmptyEntity();
        entity.emplace<Component1>(3);
        entity.emplace<Component2>(utils::String("3"));
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

TEST(ECSTest, multipleComponentView)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };
    struct Component3 { float value; };

    GE::ECSWorld world;

    {
        GE::Entity entity = world.newEmptyEntity();
        entity.emplace<Component1>(1);
    }
    {
        GE::Entity entity = world.newEmptyEntity();
        entity.emplace<Component2>(utils::String("2"));
    }
    {
        GE::Entity entity = world.newEmptyEntity();
        entity.emplace<Component1>(3);
        entity.emplace<Component2>(utils::String("3"));
    }
    {
        GE::Entity entity = world.newEmptyEntity();
        entity.emplace<Component1>(4);
        entity.emplace<Component2>(utils::String("4"));
        entity.emplace<Component3>(4.0f);
    }

    {
        GE::ECSView<Component1, Component2> view(world);
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            utils::Set<int> valuesInt;
            utils::Set<utils::String> valuesStr;
            view.onEach([&](GE::Entity, Component1& comp1, Component2& comp2) {
                valuesInt.insert(comp1.value);
                valuesStr.insert(comp2.value);
            });
        });
    }
}

TEST(ECSTest, componentEdit)
{
    struct Component1 { int value; };
    struct Component2 { utils::String value; };

    GE::ECSWorld world;

    {
        GE::Entity entity = world.newEmptyEntity();
        entity.emplace<Component1>(1);
    }
    {
        GE::Entity entity = world.newEntity();
        entity.emplace<Component2>(utils::String("2"));
    }
    {
        GE::Entity entity = world.newEntity();
        entity.emplace<Component1>(1);
        entity.emplace<Component2>(utils::String("2"));
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
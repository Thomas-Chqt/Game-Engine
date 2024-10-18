/*
 * ---------------------------------------------------
 * ECS_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 11:08:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "ECS/ECSWorld.hpp"
#include "ECS/ECSView.hpp"
#include "UtilsCPP/Set.hpp"

namespace ECS_tests
{

using EntityID = GE::ECSWorld::EntityID;

struct Component1
{
    int value = 0;

    int& val() { return value; }

    Component1() = default;
    Component1(int v) : value(v) {}
};

struct Component2
{
    int* value;

    int& val() { return *value; }

    Component2() : value(new int(0)) {}
    Component2(int v) : value(new int(v)) {}
    Component2(const Component2& cp) : value(new int(*cp.value)) {}
    Component2(Component2&& mv) : value(mv.value) { mv.value = nullptr; }
    ~Component2() { delete value; }
};

template<typename T>
class ECSTest : public testing::Test {};

using TestedComponents = ::testing::Types<Component1, Component2>;

TYPED_TEST_SUITE(ECSTest, TestedComponents);

TEST(ECSTest, newEntity)
{
    GE::ECSWorld world;

    EntityID entity1 = world.newEntityID();
    EXPECT_TRUE(world.isValidEntityID(entity1));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 0);

    EntityID entity2 = world.newEntityID();
    EXPECT_TRUE(world.isValidEntityID(entity2));
    EXPECT_NE(entity1, entity2);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 0);
    
    world.deleteEntityID(entity1);
    EXPECT_FALSE(world.isValidEntityID(entity1));
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 0);

    EntityID entity3 = world.newEntityID();
    EXPECT_TRUE(world.isValidEntityID(entity3));
    EXPECT_EQ(entity3, entity1);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 1);
    EXPECT_EQ(world.componentCount(), 0);
}

TYPED_TEST(ECSTest, oneComponent)
{
    GE::ECSWorld world;

    EntityID entity = world.newEntityID();

    EXPECT_EQ(world.emplace<TypeParam>(entity).val(), 0);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 1);
    
    EXPECT_EQ(world.get<TypeParam>(entity).val(), 0);
}

TEST(ECSTest, twoComponent)
{
    GE::ECSWorld world;

    EntityID entity = world.newEntityID();

    EXPECT_EQ(world.emplace<Component1>(entity, 1).val(), 1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 1);

    EXPECT_DEATH({
        world.emplace<Component1>(entity, 1);
    }, "");

    EXPECT_EQ(world.emplace<Component2>(entity, 2).val(), 2);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    EXPECT_DEATH({
        world.emplace<Component2>(entity, 2);
    }, "");

    EXPECT_EQ(world.get<Component1>(entity).val(), 1);
    EXPECT_EQ(world.get<Component2>(entity).val(), 2);
}

TYPED_TEST(ECSTest, deleteEntity)
{
    GE::ECSWorld world;

    EntityID entity = world.newEntityID();
    world.emplace<Component1>(entity, 1);
    world.emplace<Component2>(entity, 2);

    world.deleteEntityID(entity);

    EXPECT_EQ(world.entityCount(), 0);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 0);
}

TEST(ECSTest, removeComponent)
{
    GE::ECSWorld world;

    EntityID entity = world.newEntityID(); // 1 arch ({id})
    world.emplace<Component1>(entity, 1); // 2 arch ({id}, {id, comp1})
    world.emplace<Component2>(entity, 1); // 3 arch ({id}, {id, comp1}, {id, comp1, comp2})

    world.remove<Component1>(entity); 
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 4); // 4 arch ({id}, {id, comp1}, {id, comp1, comp2}, {id, comp2})
    EXPECT_EQ(world.componentCount(), 1);

    world.remove<Component2>(entity); 
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 4); // 4 arch ({id}, {id, comp1}, {id, comp1, comp2}, {id, comp2})
    EXPECT_EQ(world.componentCount(), 0);
}

TEST(ECSTest, multipleEntity)
{
    GE::ECSWorld world;

    EntityID entity1 = world.newEntityID();
    world.emplace<Component1>(entity1, 1);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 1);

    EntityID entity2 = world.newEntityID();
    world.emplace<Component1>(entity2, 1);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 2);

    EntityID entity3 = world.newEntityID();
    world.emplace<Component1>(entity3, 1);
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 3);
}

TEST(ECSTest, multipleEntityTwoComponent)
{
    GE::ECSWorld world;

    EntityID entity1 = world.newEntityID();
    world.emplace<Component1>(entity1, 1);
    world.emplace<Component2>(entity1, 2);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    EntityID entity2 = world.newEntityID();
    world.emplace<Component1>(entity2, 1);
    world.emplace<Component2>(entity2, 2);
    EXPECT_EQ(world.entityCount(), 2);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 4);

    EntityID entity3 = world.newEntityID();
    world.emplace<Component1>(entity3, 1);
    world.emplace<Component2>(entity3, 2);
    EXPECT_EQ(world.entityCount(), 3);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 6);
}

TYPED_TEST(ECSTest, componentEdit)
{
    GE::ECSWorld world;

    EntityID entity = world.newEntityID();
    world.emplace<TypeParam>(entity, 1);

    world.get<TypeParam>(entity).val() = 2;

    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 1);
    EXPECT_EQ(world.get<TypeParam>(entity).val(), 2);
}

TYPED_TEST(ECSTest, copyConstructor)
{
    GE::ECSWorld world1;

    EntityID entity = world1.newEntityID();
    world1.emplace<TypeParam>(entity, 1);

    GE::ECSWorld world2(world1);

    EXPECT_EQ(world1.entityCount(),                world2.entityCount());
    EXPECT_EQ(world1.archetypeCount(),             world2.archetypeCount());
    EXPECT_EQ(world1.componentCount(),             world2.componentCount());
    EXPECT_EQ(world1.get<TypeParam>(entity).val(), world2.get<TypeParam>(entity).val());

    world2.get<TypeParam>(entity).val() = 2;

    EXPECT_NE(world1.get<TypeParam>(entity).val(), world2.get<TypeParam>(entity).val());
}

TEST(ECSTest, view)
{
    GE::ECSWorld world;

    EntityID entity1 = world.newEntityID();
    world.emplace<Component1>(entity1, 1);

    EntityID entity2 = world.newEntityID();
    world.emplace<Component2>(entity2, 2);

    EntityID entity3 = world.newEntityID();
    world.emplace<Component1>(entity3, 3);
    world.emplace<Component2>(entity3, 3);

    {
        GE::ECSView<Component1> view(world);
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            utils::Set<EntityID> entities;
            utils::Set<int> values;

            view.onEach([&](GE::Entity entity, Component1& comp){ 
                entities.insert(entity.entityID());
                values.insert(comp.val());
            });

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
        });
    }
    {
        GE::ECSView<Component2> view(world);
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            utils::Set<EntityID> entities;
            utils::Set<int> values;

            view.onEach([&](GE::Entity entity, Component2& comp){ 
                entities.insert(entity.entityID());
                values.insert(comp.val());
            });

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
        });
    }
    {
        GE::ECSView<Component1, Component2> view(world);
        EXPECT_EQ(view.count(), 1);

        EXPECT_NO_THROW({
            utils::Set<EntityID> entities;
            utils::Set<int> values1;
            utils::Set<int> values2;

            view.onEach([&](GE::Entity entity, Component1& comp1, Component2& comp2){ 
                entities.insert(entity.entityID());
                values1.insert(comp1.val());
                values2.insert(comp2.val());
            });

            EXPECT_EQ(entities.size(), 1);
            EXPECT_EQ(values1.size(), 1);
            EXPECT_EQ(values2.size(), 1);
        });
    }
}

}
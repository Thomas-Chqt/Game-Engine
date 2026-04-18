/*
 * ---------------------------------------------------
 * ECS_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 11:08:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/ECSView.hpp"

#include <set>
#include <vector>
#include <ranges>

namespace GE_tests
{

using EntityID = GE::ECSWorld::EntityID;

struct Component1
{
    int value = 0;

    int& val() { return value; }
    const int& val() const { return value; }

    Component1() = default;
    Component1(int v) : value(v) {}
};

struct Component2
{
    int* value;

    int& val() { return *value; }
    const int& val() const { return *value; }

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

    #ifdef NDEBUG
    EXPECT_ANY_THROW({
        world.emplace<Component1>(entity, 1);
    });
    #else
    EXPECT_DEATH({
        world.emplace<Component1>(entity, 1);
    }, "");
    #endif

    EXPECT_EQ(world.emplace<Component2>(entity, 2).val(), 2);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 3);
    EXPECT_EQ(world.componentCount(), 2);

    #ifdef NDEBUG
    EXPECT_ANY_THROW({
        world.emplace<Component2>(entity, 2);
    });
    #else
    EXPECT_DEATH({
        world.emplace<Component2>(entity, 2);
    }, "");
    #endif

    EXPECT_EQ(world.get<Component1>(entity).val(), 1);
    EXPECT_EQ(world.get<Component2>(entity).val(), 2);
}

TYPED_TEST(ECSTest, deleteEntityOneComp)
{
    GE::ECSWorld world;

    EntityID entity = world.newEntityID();
    world.emplace<TypeParam>(entity, 1);

    world.deleteEntityID(entity);

    EXPECT_EQ(world.entityCount(), 0);
    EXPECT_EQ(world.archetypeCount(), 2);
    EXPECT_EQ(world.componentCount(), 0);
}

TEST(ECSTest, deleteEntityTwoComp)
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

TYPED_TEST(ECSTest, removeOneComponent)
{
    GE::ECSWorld world;

    EntityID entity = world.newEntityID(); // 1 arch ({id})
    world.emplace<TypeParam>(entity, 1); // 2 arch ({id}, {id, TypeParam})

    world.remove<TypeParam>(entity);
    EXPECT_EQ(world.entityCount(), 1);
    EXPECT_EQ(world.archetypeCount(), 2); // 2 arch ({id}, {id, TypeParam})
    EXPECT_EQ(world.componentCount(), 0);
}

TEST(ECSTest, removeTwoComponent)
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

TEST(ECSTest, worldRange)
{
    GE::ECSWorld world;

    EntityID entity1 = world.newEntityID();
    EntityID entity2 = world.newEntityID();
    EntityID entity3 = world.newEntityID();

    world.deleteEntityID(entity2);

    EXPECT_EQ(std::ranges::distance(world), 2);
    EXPECT_EQ(std::ranges::distance(world.cbegin(), world.cend()), 2);

    std::vector<EntityID> forward;
    for (EntityID id : world)
        forward.push_back(id);

    std::vector<EntityID> reverse;
    for (auto it = world.rbegin(); it != world.rend(); ++it)
        reverse.push_back(*it);

    std::vector<EntityID> constReverse;
    const GE::ECSWorld& constWorld = world;
    for (auto it = constWorld.crbegin(); it != constWorld.crend(); ++it)
        constReverse.push_back(*it);

    ASSERT_EQ(forward.size(), 2);
    ASSERT_EQ(reverse.size(), 2);
    ASSERT_EQ(constReverse.size(), 2);

    EXPECT_EQ(forward[0], entity1);
    EXPECT_EQ(forward[1], entity3);
    EXPECT_EQ(reverse[0], entity3);
    EXPECT_EQ(reverse[1], entity1);
    EXPECT_EQ(constReverse[0], entity3);
    EXPECT_EQ(constReverse[1], entity1);
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
        GE::ECSView<Component1> view = world | GE::ECSView<Component1>();
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            std::set<EntityID> entities;
            std::set<int> values;
            std::vector<EntityID> idsForward;
            std::vector<EntityID> idsReverse;

            for (auto it = view.cbegin(); it != view.cend(); ++it)
            {
                entities.insert(static_cast<EntityID>(*it));
                values.insert(GE::get<0>(*it).val());
                idsForward.push_back(static_cast<EntityID>(*it));
            }

            for (auto it = view.rbegin(); it != view.rend(); ++it)
            {
                idsReverse.push_back(static_cast<EntityID>(*it));
            }

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
            EXPECT_EQ(idsForward.size(), 2);
            EXPECT_EQ(idsReverse.size(), 2);
            EXPECT_EQ(idsForward[0], entity1);
            EXPECT_EQ(idsForward[1], entity3);
            EXPECT_EQ(idsReverse[0], entity3);
            EXPECT_EQ(idsReverse[1], entity1);
        });
    }
    {
        GE::ECSView<Component2> view = world | GE::ECSView<Component2>();
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            std::set<EntityID> entities;
            std::set<int> values;
            std::vector<EntityID> idsForward;
            std::vector<EntityID> idsReverse;

            for (auto it = view.cbegin(); it != view.cend(); ++it)
            {
                entities.insert(static_cast<EntityID>(*it));
                values.insert(GE::get<0>(*it).val());
                idsForward.push_back(static_cast<EntityID>(*it));
            }

            for (auto it = view.crbegin(); it != view.crend(); ++it)
            {
                idsReverse.push_back(static_cast<EntityID>(*it));
            }

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
            EXPECT_EQ(idsForward.size(), 2);
            EXPECT_EQ(idsReverse.size(), 2);
            EXPECT_EQ(idsForward[0], entity1);
            EXPECT_EQ(idsForward[1], entity3);
            EXPECT_EQ(idsReverse[0], entity3);
            EXPECT_EQ(idsReverse[1], entity1);
        });
    }
    {
        GE::ECSView<Component1, Component2> view = world | GE::ECSView<Component1, Component2>();
        EXPECT_EQ(view.count(), 1);

        EXPECT_NO_THROW({
            std::set<EntityID> entityIds;
            std::set<int> values1;
            std::set<int> values2;

            for (GE::Entity entity : view)
                entityIds.insert(entity.entityId);

            for (EntityID id : world | GE::ECSView<Component1, Component2>())
                entityIds.insert(id);

            for (auto [component1, component2] : world | GE::ECSView<Component1, Component2>())
            {
                values1.insert(component1.val());
                values2.insert(component2.val());
            }

            EXPECT_EQ(entityIds.size(), 1);
            EXPECT_EQ(values1.size(), 1);
            EXPECT_EQ(values2.size(), 1);
        });
    }
}

TEST(ECSTest, constView)
{
    GE::ECSWorld world;

    EntityID entity1 = world.newEntityID();
    world.emplace<Component1>(entity1, 1);

    EntityID entity2 = world.newEntityID();
    world.emplace<Component2>(entity2, 2);

    EntityID entity3 = world.newEntityID();
    world.emplace<Component1>(entity3, 3);
    world.emplace<Component2>(entity3, 3);

    const GE::ECSWorld& constWorld = world;

    {
        GE::const_ECSView<Component1> view = constWorld | GE::const_ECSView<Component1>();
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            std::set<EntityID> entities;
            std::set<int> values;
            std::vector<EntityID> idsForward;
            std::vector<EntityID> idsReverse;

            for (auto it = view.cbegin(); it != view.cend(); ++it)
            {
                entities.insert(static_cast<EntityID>(*it));
                values.insert(GE::get<0>(*it).val());
                idsForward.push_back(static_cast<EntityID>(*it));
            }

            for (auto it = view.crbegin(); it != view.crend(); ++it)
            {
                idsReverse.push_back(static_cast<EntityID>(*it));
            }

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
            EXPECT_EQ(idsForward.size(), 2);
            EXPECT_EQ(idsReverse.size(), 2);
            EXPECT_EQ(idsForward[0], entity1);
            EXPECT_EQ(idsForward[1], entity3);
            EXPECT_EQ(idsReverse[0], entity3);
            EXPECT_EQ(idsReverse[1], entity1);
        });
    }
    {
        GE::const_ECSView<Component2> view = constWorld | GE::const_ECSView<Component2>();
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            std::set<EntityID> entities;
            std::set<int> values;

            for (auto row : view)
            {
                entities.insert(static_cast<EntityID>(row));
                values.insert(GE::get<0>(row).val());
            }

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
        });
    }
    {
        GE::const_ECSView<Component1, Component2> view = constWorld | GE::const_ECSView<Component1, Component2>();
        EXPECT_EQ(view.count(), 1);

        EXPECT_NO_THROW({
            std::set<EntityID> entityIds;
            std::set<int> values1;
            std::set<int> values2;

            for (GE::const_Entity entity : view)
                entityIds.insert(entity.entityId);

            for (EntityID id : constWorld | GE::const_ECSView<Component1, Component2>())
                entityIds.insert(id);

            for (auto [component1, component2] : constWorld | GE::const_ECSView<Component1, Component2>())
            {
                values1.insert(component1.val());
                values2.insert(component2.val());
            }

            EXPECT_EQ(entityIds.size(), 1);
            EXPECT_EQ(values1.size(), 1);
            EXPECT_EQ(values2.size(), 1);
        });
    }
}

}

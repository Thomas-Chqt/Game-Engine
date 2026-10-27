/*
 * ---------------------------------------------------
 * ECS_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/07/21 11:08:03
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/ECSRegistry.hpp"
#include "Game-Engine/ECSView.hpp"

#include <set>
#include <vector>

namespace GE_tests
{

using EntityID = GE::EntityID;

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

struct Component3
{
    int value = 0;

    int& val() { return value; }
    const int& val() const { return value; }

    Component3() = default;
    Component3(int v) : value(v) {}
};

template<typename T>
class ECSTest : public testing::Test {};

using TestedComponents = ::testing::Types<Component1, Component2>;

TYPED_TEST_SUITE(ECSTest, TestedComponents);

TEST(ECSTest, newEntity)
{
    GE::ECSRegistry registry;

    EntityID entity1 = registry.newEntityID();
    EXPECT_TRUE(registry.isValidEntityID(entity1));
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 1);
    EXPECT_EQ(registry.componentCount(), 0);

    EntityID entity2 = registry.newEntityID();
    EXPECT_TRUE(registry.isValidEntityID(entity2));
    EXPECT_NE(entity1, entity2);
    EXPECT_EQ(registry.entityCount(), 2);
    EXPECT_EQ(registry.archetypeCount(), 1);
    EXPECT_EQ(registry.componentCount(), 0);

    registry.deleteEntityID(entity1);
    EXPECT_FALSE(registry.isValidEntityID(entity1));
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 1);
    EXPECT_EQ(registry.componentCount(), 0);

    EntityID entity3 = registry.newEntityID();
    EXPECT_TRUE(registry.isValidEntityID(entity3));
    EXPECT_NE(entity3, entity1);
    EXPECT_EQ(registry.entityCount(), 2);
    EXPECT_EQ(registry.archetypeCount(), 1);
    EXPECT_EQ(registry.componentCount(), 0);
}

TYPED_TEST(ECSTest, oneComponent)
{
    GE::ECSRegistry registry;

    EntityID entity = registry.newEntityID();

    EXPECT_EQ(registry.emplace<TypeParam>(entity).val(), 0);
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 2);
    EXPECT_EQ(registry.componentCount(), 1);

    EXPECT_EQ(registry.get<TypeParam>(entity).val(), 0);
}

TEST(ECSTest, twoComponent)
{
    GE::ECSRegistry registry;

    EntityID entity = registry.newEntityID();

    EXPECT_EQ(registry.emplace<Component1>(entity, 1).val(), 1);
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 2);
    EXPECT_EQ(registry.componentCount(), 1);

#ifndef NDEBUG
    EXPECT_DEATH({
        registry.emplace<Component1>(entity, 1);
    }, "");
#endif

    EXPECT_EQ(registry.emplace<Component2>(entity, 2).val(), 2);
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 3);
    EXPECT_EQ(registry.componentCount(), 2);

#ifndef NDEBUG
    EXPECT_DEATH({
        registry.emplace<Component2>(entity, 2);
    }, "");
#endif

    EXPECT_EQ(registry.get<Component1>(entity).val(), 1);
    EXPECT_EQ(registry.get<Component2>(entity).val(), 2);
}

TYPED_TEST(ECSTest, deleteEntityOneComp)
{
    GE::ECSRegistry registry;

    EntityID entity = registry.newEntityID();
    registry.emplace<TypeParam>(entity, 1);

    registry.deleteEntityID(entity);

    EXPECT_EQ(registry.entityCount(), 0);
    EXPECT_EQ(registry.archetypeCount(), 2);
    EXPECT_EQ(registry.componentCount(), 0);
}

TEST(ECSTest, deleteEntityTwoComp)
{
    GE::ECSRegistry registry;

    EntityID entity = registry.newEntityID();
    registry.emplace<Component1>(entity, 1);
    registry.emplace<Component2>(entity, 2);

    registry.deleteEntityID(entity);

    EXPECT_EQ(registry.entityCount(), 0);
    EXPECT_EQ(registry.archetypeCount(), 3);
    EXPECT_EQ(registry.componentCount(), 0);
}

TYPED_TEST(ECSTest, removeOneComponent)
{
    GE::ECSRegistry registry;

    EntityID entity = registry.newEntityID(); // 1 arch ({id})
    registry.emplace<TypeParam>(entity, 1); // 2 arch ({id}, {id, TypeParam})

    registry.remove<TypeParam>(entity);
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 2); // 2 arch ({id}, {id, TypeParam})
    EXPECT_EQ(registry.componentCount(), 0);
}

TEST(ECSTest, removeTwoComponent)
{
    GE::ECSRegistry registry;

    EntityID entity = registry.newEntityID(); // 1 arch ({id})
    registry.emplace<Component1>(entity, 1); // 2 arch ({id}, {id, comp1})
    registry.emplace<Component2>(entity, 1); // 3 arch ({id}, {id, comp1}, {id, comp1, comp2})

    registry.remove<Component1>(entity);
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 4); // 4 arch ({id}, {id, comp1}, {id, comp1, comp2}, {id, comp2})
    EXPECT_EQ(registry.componentCount(), 1);

    registry.remove<Component2>(entity);
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 4); // 4 arch ({id}, {id, comp1}, {id, comp1, comp2}, {id, comp2})
    EXPECT_EQ(registry.componentCount(), 0);
}

TEST(ECSTest, multipleEntity)
{
    GE::ECSRegistry registry;

    EntityID entity1 = registry.newEntityID();
    registry.emplace<Component1>(entity1, 1);
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 2);
    EXPECT_EQ(registry.componentCount(), 1);

    EntityID entity2 = registry.newEntityID();
    registry.emplace<Component1>(entity2, 1);
    EXPECT_EQ(registry.entityCount(), 2);
    EXPECT_EQ(registry.archetypeCount(), 2);
    EXPECT_EQ(registry.componentCount(), 2);

    EntityID entity3 = registry.newEntityID();
    registry.emplace<Component1>(entity3, 1);
    EXPECT_EQ(registry.entityCount(), 3);
    EXPECT_EQ(registry.archetypeCount(), 2);
    EXPECT_EQ(registry.componentCount(), 3);
}

TEST(ECSTest, multipleEntityTwoComponent)
{
    GE::ECSRegistry registry;

    EntityID entity1 = registry.newEntityID();
    registry.emplace<Component1>(entity1, 1);
    registry.emplace<Component2>(entity1, 2);
    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 3);
    EXPECT_EQ(registry.componentCount(), 2);

    EntityID entity2 = registry.newEntityID();
    registry.emplace<Component1>(entity2, 1);
    registry.emplace<Component2>(entity2, 2);
    EXPECT_EQ(registry.entityCount(), 2);
    EXPECT_EQ(registry.archetypeCount(), 3);
    EXPECT_EQ(registry.componentCount(), 4);

    EntityID entity3 = registry.newEntityID();
    registry.emplace<Component1>(entity3, 1);
    registry.emplace<Component2>(entity3, 2);
    EXPECT_EQ(registry.entityCount(), 3);
    EXPECT_EQ(registry.archetypeCount(), 3);
    EXPECT_EQ(registry.componentCount(), 6);
}

TEST(ECSTest, registryRange)
{
    GE::ECSRegistry registry;

    EntityID entity1 = registry.newEntityID();
    EntityID entity2 = registry.newEntityID();
    EntityID entity3 = registry.newEntityID();

    registry.deleteEntityID(entity2);

    EXPECT_EQ(std::ranges::distance(registry), 2);

    std::vector<EntityID> forward;
    for (EntityID id : registry)
        forward.push_back(id);

    ASSERT_EQ(forward.size(), 2);

    EXPECT_EQ(std::set<EntityID>(forward.begin(), forward.end()), (std::set<EntityID>{entity1, entity3}));
}

TYPED_TEST(ECSTest, componentEdit)
{
    GE::ECSRegistry registry;

    EntityID entity = registry.newEntityID();
    registry.emplace<TypeParam>(entity, 1);

    registry.get<TypeParam>(entity).val() = 2;

    EXPECT_EQ(registry.entityCount(), 1);
    EXPECT_EQ(registry.archetypeCount(), 2);
    EXPECT_EQ(registry.componentCount(), 1);
    EXPECT_EQ(registry.get<TypeParam>(entity).val(), 2);
}

TYPED_TEST(ECSTest, copyConstructor)
{
    GE::ECSRegistry registry1;

    EntityID entity = registry1.newEntityID();
    registry1.emplace<TypeParam>(entity, 1);

    GE::ECSRegistry registry2(registry1);

    EXPECT_EQ(registry1.entityCount(),                registry2.entityCount());
    EXPECT_EQ(registry1.archetypeCount(),             registry2.archetypeCount());
    EXPECT_EQ(registry1.componentCount(),             registry2.componentCount());
    EXPECT_EQ(registry1.get<TypeParam>(entity).val(), registry2.get<TypeParam>(entity).val());

    registry2.get<TypeParam>(entity).val() = 2;

    EXPECT_NE(registry1.get<TypeParam>(entity).val(), registry2.get<TypeParam>(entity).val());
}

TEST(ECSTest, view)
{
    GE::ECSRegistry registry;

    EntityID entity1 = registry.newEntityID();
    registry.emplace<Component1>(entity1, 1);

    EntityID entity2 = registry.newEntityID();
    registry.emplace<Component2>(entity2, 2);

    EntityID entity3 = registry.newEntityID();
    registry.emplace<Component1>(entity3, 3);
    registry.emplace<Component2>(entity3, 3);

    {
        GE::ECSView<Component1> view = registry | GE::ECSView<Component1>();
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            std::set<EntityID> entities;
            std::set<int> values;
            std::set<int> structuredValues;

            for (auto row : view)
                entities.insert(static_cast<EntityID>(row));

            for (auto [component1] : view)
            {
                values.insert(component1.val());
                structuredValues.insert(component1.val());
            }

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
            EXPECT_EQ(structuredValues.size(), 2);
        });
    }
    {
        GE::ECSView<Component2> view = registry | GE::ECSView<Component2>();
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            std::set<EntityID> entities;
            std::set<int> values;
            std::set<int> structuredValues;

            for (auto row : view)
                entities.insert(static_cast<EntityID>(row));

            for (auto [component2] : view)
            {
                values.insert(component2.val());
                structuredValues.insert(component2.val());
            }

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
            EXPECT_EQ(structuredValues.size(), 2);
        });
    }
    {
        GE::ECSView<Component1, Component2> view = registry | GE::ECSView<Component1, Component2>();
        EXPECT_EQ(view.count(), 1);

        EXPECT_NO_THROW({
            std::set<EntityID> entityIds;
            std::set<int> values1;
            std::set<int> values2;
            std::set<int> structuredValues1;
            std::set<int> structuredValues2;

            for (auto row : view)
                entityIds.insert(static_cast<EntityID>(row));

            for (auto row : registry | GE::ECSView<Component1, Component2>())
                entityIds.insert(static_cast<EntityID>(row));

            for (auto [component1, component2] : registry | GE::ECSView<Component1, Component2>())
            {
                values1.insert(component1.val());
                values2.insert(component2.val());
                structuredValues1.insert(component1.val());
                structuredValues2.insert(component2.val());
            }

            EXPECT_EQ(entityIds.size(), 1);
            EXPECT_EQ(values1.size(), 1);
            EXPECT_EQ(values2.size(), 1);
            EXPECT_EQ(structuredValues1.size(), 1);
            EXPECT_EQ(structuredValues2.size(), 1);
        });
    }
}

TEST(ECSTest, viewWithoutComponent)
{
    GE::ECSRegistry registry;

    EntityID noComponent = registry.newEntityID();

    EntityID component1Only = registry.newEntityID();
    registry.emplace<Component1>(component1Only, 1);

    EntityID component2Only = registry.newEntityID();
    registry.emplace<Component2>(component2Only, 2);

    EntityID component1And2 = registry.newEntityID();
    registry.emplace<Component1>(component1And2, 12);
    registry.emplace<Component2>(component1And2, 12);

    EntityID component1And3 = registry.newEntityID();
    registry.emplace<Component1>(component1And3, 13);
    registry.emplace<Component3>(component1And3, 13);

    EntityID component1And2And3 = registry.newEntityID();
    registry.emplace<Component1>(component1And2And3, 123);
    registry.emplace<Component2>(component1And2And3, 123);
    registry.emplace<Component3>(component1And2And3, 123);

    {
        GE::ECSView<Component1> view = registry | GE::ECSView<Component1>().without<Component2>();
        EXPECT_EQ(view.count(), 2);

        std::set<EntityID> entityIds;
        std::set<int> values;

        for (auto row : view)
            entityIds.insert(static_cast<EntityID>(row));

        for (auto [component1] : view)
            values.insert(component1.val());

        EXPECT_TRUE(entityIds.contains(component1Only));
        EXPECT_TRUE(entityIds.contains(component1And3));
        EXPECT_FALSE(entityIds.contains(noComponent));
        EXPECT_FALSE(entityIds.contains(component2Only));
        EXPECT_FALSE(entityIds.contains(component1And2));
        EXPECT_FALSE(entityIds.contains(component1And2And3));
        EXPECT_EQ(values, std::set<int>({1, 13}));
    }
    {
        GE::ECSView<Component1, Component2> view = registry | GE::ECSView<Component1, Component2>().without<Component3>();
        EXPECT_EQ(view.count(), 1);

        std::set<EntityID> entityIds;
        std::set<int> values1;
        std::set<int> values2;

        for (auto row : view)
            entityIds.insert(static_cast<EntityID>(row));

        for (auto [component1, component2] : view)
        {
            values1.insert(component1.val());
            values2.insert(component2.val());
        }

        EXPECT_TRUE(entityIds.contains(component1And2));
        EXPECT_FALSE(entityIds.contains(component1And2And3));
        EXPECT_EQ(values1, std::set<int>({12}));
        EXPECT_EQ(values2, std::set<int>({12}));
    }
    {
        GE::ECSView<Component1> view = registry | GE::ECSView<Component1>().without<Component2, Component3>();
        EXPECT_EQ(view.count(), 1);

        std::set<EntityID> entityIds;
        for (auto row : view)
            entityIds.insert(static_cast<EntityID>(row));

        EXPECT_TRUE(entityIds.contains(component1Only));
        EXPECT_FALSE(entityIds.contains(component1And2));
        EXPECT_FALSE(entityIds.contains(component1And3));
        EXPECT_FALSE(entityIds.contains(component1And2And3));
    }
}

TEST(ECSTest, constView)
{
    GE::ECSRegistry registry;

    EntityID entity1 = registry.newEntityID();
    registry.emplace<Component1>(entity1, 1);

    EntityID entity2 = registry.newEntityID();
    registry.emplace<Component2>(entity2, 2);

    EntityID entity3 = registry.newEntityID();
    registry.emplace<Component1>(entity3, 3);
    registry.emplace<Component2>(entity3, 3);

    const GE::ECSRegistry& constRegistry = registry;

    {
        GE::const_ECSView<Component1> view = constRegistry | GE::const_ECSView<Component1>();
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            std::set<EntityID> entities;
            std::set<int> values;
            std::set<int> structuredValues;

            for (auto row : view)
                entities.insert(static_cast<EntityID>(row));

            for (auto [component1] : view)
            {
                values.insert(component1.val());
                structuredValues.insert(component1.val());
            }

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
            EXPECT_EQ(structuredValues.size(), 2);
        });
    }
    {
        GE::const_ECSView<Component2> view = constRegistry | GE::const_ECSView<Component2>();
        EXPECT_EQ(view.count(), 2);

        EXPECT_NO_THROW({
            std::set<EntityID> entities;
            std::set<int> values;
            std::set<int> structuredValues;

            for (auto row : view)
                entities.insert(static_cast<EntityID>(row));

            for (auto [component2] : view)
            {
                values.insert(component2.val());
                structuredValues.insert(component2.val());
            }

            EXPECT_EQ(entities.size(), 2);
            EXPECT_EQ(values.size(), 2);
            EXPECT_EQ(structuredValues.size(), 2);
        });
    }
    {
        GE::const_ECSView<Component1, Component2> view = constRegistry | GE::const_ECSView<Component1, Component2>();
        EXPECT_EQ(view.count(), 1);

        EXPECT_NO_THROW({
            std::set<EntityID> entityIds;
            std::set<int> values1;
            std::set<int> values2;
            std::set<int> structuredValues1;
            std::set<int> structuredValues2;

            for (auto row : view)
                entityIds.insert(static_cast<EntityID>(row));

            for (auto row : constRegistry | GE::const_ECSView<Component1, Component2>())
                entityIds.insert(static_cast<EntityID>(row));

            for (auto [component1, component2] : constRegistry | GE::const_ECSView<Component1, Component2>())
            {
                values1.insert(component1.val());
                values2.insert(component2.val());
                structuredValues1.insert(component1.val());
                structuredValues2.insert(component2.val());
            }

            EXPECT_EQ(entityIds.size(), 1);
            EXPECT_EQ(values1.size(), 1);
            EXPECT_EQ(values2.size(), 1);
            EXPECT_EQ(structuredValues1.size(), 1);
            EXPECT_EQ(structuredValues2.size(), 1);
        });
    }
}

TEST(ECSTest, constViewWithoutComponent)
{
    GE::ECSRegistry registry;

    EntityID component1Only = registry.newEntityID();
    registry.emplace<Component1>(component1Only, 1);

    EntityID component1And2 = registry.newEntityID();
    registry.emplace<Component1>(component1And2, 12);
    registry.emplace<Component2>(component1And2, 12);

    const GE::ECSRegistry& constRegistry = registry;
    GE::const_ECSView<Component1> view = constRegistry | GE::const_ECSView<Component1>().without<Component2>();
    EXPECT_EQ(view.count(), 1);

    std::set<EntityID> entityIds;
    std::set<int> values;

    for (auto row : view)
        entityIds.insert(static_cast<EntityID>(row));

    for (auto [component1] : view)
        values.insert(component1.val());

    EXPECT_TRUE(entityIds.contains(component1Only));
    EXPECT_FALSE(entityIds.contains(component1And2));
    EXPECT_EQ(values, std::set<int>({1}));
}

TEST(ECSTest, emptyRegistryAndView)
{
    GE::ECSRegistry registry;
    EXPECT_EQ(registry.entityCount(), 0);
    EXPECT_EQ(registry.componentCount(), 0);
    EXPECT_EQ(registry.begin(), registry.end());
    EXPECT_EQ((registry | GE::ECSView<Component1>()).count(), 0);
    GE::ECSView<Component1> unbound;
    EXPECT_EQ(unbound.count(), 0);
    EXPECT_EQ(unbound.begin(), unbound.end());
}

TYPED_TEST(ECSTest, denseRowsSurviveMovesAndDeletion)
{
    GE::ECSRegistry registry;
    std::vector<EntityID> ids;
    for (int i = 0; i < 48; ++i) {
        const auto id = registry.newEntityID();
        ids.push_back(id);
        registry.emplace<TypeParam>(id, i);
    }
    for (int i = 0; i < 48; i += 2)
        registry.emplace<Component3>(ids[i], i + 100);
    for (int i = 0; i < 48; i += 4)
        registry.remove<TypeParam>(ids[i]);
    for (int i = 0; i < 48; i += 4)
        registry.emplace<TypeParam>(ids[i], i);
    for (int i = 0; i < 48; i += 3)
        registry.deleteEntityID(ids[i]);

    size_t count = 0;
    for (int i = 0; i < 48; ++i) {
        if (i % 3 == 0) {
            EXPECT_FALSE(registry.isValidEntityID(ids[i]));
            continue;
        }
        ++count;
        EXPECT_EQ(registry.get<TypeParam>(ids[i]).val(), i);
        EXPECT_EQ(registry.get<EntityID>(ids[i]), ids[i]);
        if (i % 2 == 0)
            EXPECT_EQ(registry.get<Component3>(ids[i]).val(), i + 100);
    }
    EXPECT_EQ(registry.entityCount(), count);
    EXPECT_EQ((registry | GE::ECSView<TypeParam>()).count(), count);
    for (const auto& row : registry | GE::ECSView<TypeParam>())
        EXPECT_EQ(row.template get<0>().val(), registry.get<TypeParam>(row.entityId).val());
}

TYPED_TEST(ECSTest, copyAssignmentAndMovePreserveStorage)
{
    GE::ECSRegistry original;
    const auto id = original.newEntityID();
    original.emplace<TypeParam>(id, 10);
    GE::ECSRegistry copy;
    copy = original;
    copy.get<TypeParam>(id).val() = 20;
    EXPECT_EQ(original.get<TypeParam>(id).val(), 10);
    copy.remove<TypeParam>(id);
    copy.emplace<TypeParam>(id, 30);
    GE::ECSRegistry moved(std::move(copy));
    EXPECT_EQ(moved.get<TypeParam>(id).val(), 30);
    moved.deleteEntityID(id);
    moved.registerEntityID(id);
    EXPECT_EQ(moved.get<EntityID>(id), id);
    EXPECT_EQ(moved.componentCount(), 0);
}

TEST(ECSTest, constAccessAndInternalIdentity)
{
    GE::ECSRegistry registry;
    const auto id = registry.newEntityID();
    registry.emplace<Component1>(id, 4);
    const auto& constRegistry = registry;
    static_assert(std::is_same_v<decltype(registry.get<EntityID>(id)), const EntityID&>);
    static_assert(std::is_same_v<decltype(constRegistry.get<Component1>(id)), const Component1&>);
    for (auto [component] : constRegistry | GE::const_ECSView<Component1>()) {
        static_assert(std::is_same_v<decltype(component), const Component1&>);
        EXPECT_EQ(component.val(), 4);
    }
}

TEST(ECSTest, viewIteratesDenseRowsAfterMovesAndDeletion)
{
    GE::ECSRegistry registry;
    std::vector<EntityID> ids;
    for (int i = 0; i < 9; ++i) {
        const auto id = registry.newEntityID();
        ids.push_back(id);
        registry.emplace<Component1>(id, i);
        registry.emplace<Component3>(id, i + 100);
    }
    registry.deleteEntityID(ids[2]);
    registry.remove<Component3>(ids[1]);
    registry.emplace<Component3>(ids[1], 101);

    const std::vector<EntityID> expected{ids[0], ids[7], ids[8], ids[3], ids[4], ids[5], ids[6], ids[1]};
    const auto view = registry | GE::ECSView<Component1, Component3>();
    size_t rowIndex = 0;
    Component1* previous1 = nullptr;
    Component3* previous3 = nullptr;
    for (auto row : view) {
        ASSERT_LT(rowIndex, expected.size());
        EXPECT_EQ(row.entityId, expected[rowIndex++]);
        auto& [component1, component3] = row;
        EXPECT_EQ(component3.val(), component1.val() + 100);
        EXPECT_EQ(&component1, &registry.get<Component1>(row.entityId));
        EXPECT_EQ(&component3, &registry.get<Component3>(row.entityId));
        if (previous1) {
            EXPECT_EQ(&component1, previous1 + 1);
            EXPECT_EQ(&component3, previous3 + 1);
        }
        previous1 = &component1;
        previous3 = &component3;
        component1.val() += 10;
        EXPECT_EQ(registry.get<Component1>(row.entityId).val(), component3.val() - 90);
    }
    EXPECT_EQ(rowIndex, expected.size());

    const auto& constRegistry = registry;
    for (auto [id, component] : constRegistry | GE::const_ECSView<EntityID, Component1>()) {
        static_assert(std::is_same_v<decltype(id), const EntityID&>);
        static_assert(std::is_same_v<decltype(component), const Component1&>);
        EXPECT_EQ(&component, &constRegistry.get<Component1>(id));
    }
}

TEST(ECSTest, viewIteratorCopiesAndPostIncrementCrossArchetypes)
{
    GE::ECSRegistry registry;
    const auto first = registry.newEntityID();
    registry.emplace<Component1>(first, 1);
    const auto second = registry.newEntityID();
    registry.emplace<Component1>(second, 2);
    registry.emplace<Component3>(second, 3);
    const auto removed = registry.newEntityID();
    registry.emplace<Component1>(removed, 4);
    registry.emplace<Component2>(removed, 4);
    registry.deleteEntityID(removed); // Leave an empty matching archetype.
    const auto excluded = registry.newEntityID();
    registry.emplace<Component2>(excluded, 5);

    using View = GE::ECSView<Component1>;
    static_assert(std::ranges::common_range<View>);
    static_assert(std::input_iterator<View::iterator>);
    const auto view = registry | View();
    auto it = view.begin();
    auto copy = it;
    const auto previous = it++;
    EXPECT_EQ(previous, copy);
    EXPECT_EQ((*previous).entityId, (*copy).entityId);
    ASSERT_NE(it, view.end());
    EXPECT_NE(it, copy);
    const auto next = it++;
    EXPECT_EQ(it, view.end());
    EXPECT_EQ((std::set<EntityID>{(*previous).entityId, (*next).entityId}), (std::set<EntityID>{first, second}));
    EXPECT_EQ(std::ranges::distance(view), 2);
    EXPECT_EQ(copy, view.begin());
}

}

#include "Game-Engine/ECSRegistry.hpp"
#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Script.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace GE_tests
{

struct InsertionReference
{
    GE::EntityID target;
};

struct InsertionValue
{
    std::vector<int> values;
};

struct InsertionCopyState
{
    inline static int attempts = 0;
    inline static int failAt = 0;
    inline static int alive = 0;
};

template<int Tag>
struct InsertionThrowingCopy
{
    int value;

    explicit InsertionThrowingCopy(int value) : value(value) { ++InsertionCopyState::alive; }
    InsertionThrowingCopy(const InsertionThrowingCopy& source) : value(source.value)
    {
        if (++InsertionCopyState::attempts == InsertionCopyState::failAt)
            throw std::runtime_error("insertion copy failed");
        ++InsertionCopyState::alive;
    }
    InsertionThrowingCopy(InsertionThrowingCopy&& source) noexcept : value(source.value) { ++InsertionCopyState::alive; }
    ~InsertionThrowingCopy() { --InsertionCopyState::alive; }

    InsertionThrowingCopy& operator=(const InsertionThrowingCopy&) = delete;
    InsertionThrowingCopy& operator=(InsertionThrowingCopy&&) = delete;
};

}

namespace GE
{

template<>
struct ComponentCloneTraits<GE_tests::InsertionReference>
{
    static GE_tests::InsertionReference clone(const GE_tests::InsertionReference& source, const EntityIDMap& ids)
    {
        return {ComponentCloneTraits<EntityID>::clone(source.target, ids)};
    }
};

}

namespace GE_tests
{

TEST(ECSRegistryInsertionTest, emptySourceIsANoOp)
{
    GE::ECSRegistry source;
    GE::ECSRegistry destination;
    EXPECT_TRUE(destination.insert(source).empty());
    EXPECT_EQ(destination.entityCount(), 0);
    EXPECT_EQ(destination.archetypeCount(), 0);
    const auto existing = destination.newEntityID();
    destination.emplace<GE::NameComponent>(existing, "existing");
    const auto archetypes = destination.archetypeCount();
    EXPECT_TRUE(destination.insert(source).empty());
    EXPECT_EQ(destination.entityCount(), 1);
    EXPECT_EQ(destination.archetypeCount(), archetypes);
    EXPECT_EQ(destination.get<GE::NameComponent>(existing).name, "existing");
}

TEST(ECSRegistryInsertionTest, copiesAllArchetypesAndCanInsertTheSameSourceTwice)
{
    GE::ECSRegistry source;
    const auto empty = source.newEntityID();
    const auto named = source.newEntityID();
    source.emplace<GE::NameComponent>(named, "named");
    source.emplace<InsertionValue>(named, std::vector<int>{1, 2, 3});
    const auto mesh = source.newEntityID();
    source.emplace<GE::MeshComponent>(mesh, 42);

    // Existing source UUIDs may already be present in the destination.
    GE::ECSRegistry destination = source;
    const auto first = destination.insert(source);
    const auto second = destination.insert(source);
    ASSERT_EQ(first.size(), 3);
    ASSERT_EQ(second.size(), 3);
    EXPECT_EQ(source.entityCount(), 3);
    EXPECT_EQ(destination.entityCount(), 9);
    EXPECT_EQ(destination.componentCount(), source.componentCount() * 3);

    std::set<GE::EntityID> uniqueIds;
    for (auto id : source)
        uniqueIds.insert(id);
    for (const auto& mapping : {first, second}) {
        for (const auto& [original, copy] : mapping) {
            EXPECT_NE(copy, original);
            EXPECT_FALSE(copy.is_nil());
            EXPECT_TRUE(uniqueIds.insert(copy).second);
            EXPECT_TRUE(destination.isValidEntityID(copy));
            EXPECT_EQ(destination.get<GE::EntityID>(copy), copy);
        }
        EXPECT_FALSE(destination.has<GE::NameComponent>(mapping.at(empty)));
        EXPECT_FALSE(destination.has<GE::MeshComponent>(mapping.at(empty)));
        EXPECT_EQ(destination.get<GE::MeshComponent>(mapping.at(mesh)).id, 42);
        EXPECT_EQ(destination.get<InsertionValue>(mapping.at(named)).values, (std::vector<int>{1, 2, 3}));
    }

    destination.get<GE::NameComponent>(first.at(named)).name = "changed";
    destination.get<InsertionValue>(first.at(named)).values[0] = 99;
    EXPECT_EQ(source.get<GE::NameComponent>(named).name, "named");
    EXPECT_EQ(destination.get<GE::NameComponent>(named).name, "named");
    EXPECT_EQ(destination.get<GE::NameComponent>(second.at(named)).name, "named");
    EXPECT_EQ(source.get<InsertionValue>(named).values[0], 1);
    EXPECT_EQ(destination.get<InsertionValue>(second.at(named)).values[0], 1);
}

TEST(ECSRegistryInsertionTest, insertsNewArchetypesIntoAnEmptyRegistry)
{
    GE::ECSRegistry source;
    const auto id = source.newEntityID();
    source.emplace<InsertionValue>(id, std::vector<int>{4, 5});
    source.emplace<GE::NameComponent>(id, "new archetype");
    GE::ECSRegistry destination;
    const auto ids = destination.insert(source);
    EXPECT_EQ(destination.entityCount(), 1);
    EXPECT_EQ(destination.componentCount(), 2);
    EXPECT_EQ(destination.get<InsertionValue>(ids.at(id)).values, (std::vector<int>{4, 5}));
    destination.remove<InsertionValue>(ids.at(id));
    destination.emplace<GE::LightComponent>(ids.at(id));
    EXPECT_EQ(destination.get<GE::NameComponent>(ids.at(id)).name, "new archetype");
    destination.deleteEntityID(ids.at(id));
    EXPECT_EQ(destination.entityCount(), 0);
    EXPECT_EQ(source.entityCount(), 1);
}

TEST(ECSRegistryInsertionTest, remapsHierarchyAndCustomReferencesWithoutReparentingRoots)
{
    GE::ECSRegistry source;
    GE::Entity root{&source, source.newEntityID()};
    GE::Entity first{&source, source.newEntityID()};
    GE::Entity second{&source, source.newEntityID()};
    GE::Entity otherRoot{&source, source.newEntityID()};
    root.emplace<GE::TransformComponent>().position = {1, 2, 3};
    first.emplace<GE::TransformComponent>().position = {4, 5, 6};
    second.emplace<GE::TransformComponent>();
    root.addChild(first);
    root.addChild(second);
    root.updateTransformHierarchy();
    first.emplace<InsertionReference>(otherRoot.entityId);
    otherRoot.emplace<GE::ChildrenComponent>();
    otherRoot.emplace<GE::ParentComponent>();

    GE::ECSRegistry destination;
    const auto existing = destination.newEntityID();
    destination.emplace<GE::ChildrenComponent>(existing);
    const auto ids = destination.insert(source);
    EXPECT_FALSE(destination.has<GE::ParentComponent>(ids.at(root.entityId)));
    const auto& copiedParent = destination.get<GE::ParentComponent>(ids.at(first.entityId));
    EXPECT_EQ(copiedParent.parent, ids.at(root.entityId));
    EXPECT_EQ(copiedParent.nextChild, ids.at(second.entityId));
    EXPECT_EQ(destination.get<GE::ParentComponent>(ids.at(second.entityId)).parent, ids.at(root.entityId));
    EXPECT_FALSE(destination.get<GE::ParentComponent>(ids.at(second.entityId)).nextChild);
    EXPECT_EQ(destination.get<GE::ChildrenComponent>(ids.at(root.entityId)).firstChild, ids.at(first.entityId));
    EXPECT_EQ(destination.get<InsertionReference>(ids.at(first.entityId)).target, ids.at(otherRoot.entityId));
    EXPECT_FALSE(destination.get<GE::ChildrenComponent>(ids.at(otherRoot.entityId)).firstChild);
    EXPECT_FALSE(destination.get<GE::ParentComponent>(ids.at(otherRoot.entityId)).parent);
    EXPECT_FALSE(destination.get<GE::ParentComponent>(ids.at(otherRoot.entityId)).nextChild);
    EXPECT_FALSE(destination.get<GE::ChildrenComponent>(existing).firstChild);
    EXPECT_EQ(destination.get<GE::TransformComponent>(ids.at(root.entityId)).position, glm::vec3(1, 2, 3));
    EXPECT_EQ(destination.get<GE::TransformComponent>(ids.at(first.entityId)).position, glm::vec3(4, 5, 6));
    EXPECT_EQ(destination.get<GE::TransformComponent>(ids.at(first.entityId)).worldTransform,
              source.get<GE::TransformComponent>(first.entityId).worldTransform);
    EXPECT_EQ(source.get<GE::ParentComponent>(first.entityId).parent, root.entityId);
    EXPECT_EQ(source.get<InsertionReference>(first.entityId).target, otherRoot.entityId);
}

TEST(ECSRegistryInsertionTest, insertionClearsScriptsButOrdinaryCopiesKeepIdsAndValues)
{
    GE::ECSRegistry source;
    const auto id = source.newEntityID();
    auto instance = std::make_shared<GE::Script>();
    source.emplace<GE::ScriptComponent>(id, "script",
        std::vector<std::pair<std::string, GE::VScriptValue>>{{"speed", 3.0f}}, instance);
    GE::ECSRegistry destination;
    const auto ids = destination.insert(source);
    const auto& inserted = destination.get<GE::ScriptComponent>(ids.at(id));
    EXPECT_EQ(inserted.name, "script");
    EXPECT_EQ(inserted.parameters, source.get<GE::ScriptComponent>(id).parameters);
    EXPECT_FALSE(inserted.instance);
    EXPECT_EQ(source.get<GE::ScriptComponent>(id).instance, instance);

    const GE::ECSRegistry snapshot(source);
    EXPECT_TRUE(snapshot.isValidEntityID(id));
    EXPECT_EQ(snapshot.get<GE::EntityID>(id), id);
    EXPECT_EQ(snapshot.get<GE::ScriptComponent>(id).instance, instance);
    GE::ECSRegistry assigned;
    assigned = source;
    EXPECT_EQ(assigned.get<GE::ScriptComponent>(id).instance, instance);
    EXPECT_FALSE(assigned.isValidEntityID(ids.at(id)));
}

TEST(ECSRegistryInsertionTest, bulkCopiesKeepColumnsAlignedAfterSourceRowSwaps)
{
    GE::ECSRegistry source;
    std::vector<GE::EntityID> originals;
    for (int value = 0; value < 128; ++value) {
        const auto id = source.newEntityID();
        originals.push_back(id);
        source.emplace<GE::NameComponent>(id, std::to_string(value));
        source.emplace<InsertionValue>(id, std::vector<int>{value, value * 2});
        if (value % 3 == 0)
            source.emplace<GE::MeshComponent>(id, value);
    }
    // Moving and erasing entities changes dense column order independently of
    // both UUID hash maps. All columns must still follow the copied ID column.
    source.deleteEntityID(originals[7]);
    source.remove<GE::MeshComponent>(originals[6]);
    source.emplace<GE::MeshComponent>(originals[5], 5);

    GE::ECSRegistry destination;
    const auto existing = destination.newEntityID();
    destination.emplace<GE::NameComponent>(existing, "existing");
    destination.emplace<InsertionValue>(existing, std::vector<int>{-1});
    for (int repeat = 0; repeat < 2; ++repeat) {
        const auto ids = destination.insert(source);
        EXPECT_EQ(ids.size(), 127);
        EXPECT_EQ(destination.entityCount(), 1 + (repeat + 1) * 127);
        for (const auto& [original, copy] : ids) {
            EXPECT_EQ(destination.get<GE::EntityID>(copy), copy);
            EXPECT_EQ(destination.get<GE::NameComponent>(copy).name, source.get<GE::NameComponent>(original).name);
            EXPECT_EQ(destination.get<InsertionValue>(copy).values, source.get<InsertionValue>(original).values);
            EXPECT_EQ(destination.has<GE::MeshComponent>(copy), source.has<GE::MeshComponent>(original));
            if (source.has<GE::MeshComponent>(original))
                EXPECT_EQ(destination.get<GE::MeshComponent>(copy).id, source.get<GE::MeshComponent>(original).id);
        }
    }
    EXPECT_EQ(destination.get<GE::NameComponent>(existing).name, "existing");
    EXPECT_EQ(destination.get<InsertionValue>(existing).values, std::vector<int>{-1});
}

TEST(ECSRegistryInsertionTest, throwingCopiesRollBackCompleteAndPartialEntities)
{
    using First = InsertionThrowingCopy<0>;
    using Second = InsertionThrowingCopy<1>;
    static_assert(GE::Component<First>);
    EXPECT_EQ(InsertionCopyState::alive, 0);
    {
        GE::ECSRegistry source;
        for (int index = 0; index < 3; ++index) {
            const auto id = source.newEntityID();
            source.emplace<First>(id, index);
            source.emplace<Second>(id, index + 10);
        }
        for (int index = 0; index < 2; ++index)
            source.emplace<First>(source.newEntityID(), index + 20);
        GE::ECSRegistry destination;
        const auto existing = destination.newEntityID();
        destination.emplace<First>(existing, 100);
        destination.emplace<Second>(existing, 200);
        const auto named = destination.newEntityID();
        destination.emplace<GE::NameComponent>(named, "existing");
        const auto alive = InsertionCopyState::alive;

        // Fail throughout columns of two archetypes: partial columns, complete
        // columns, and previously completed archetypes must all be rolled back.
        for (int failAt = 1; failAt <= 8; ++failAt) {
            InsertionCopyState::attempts = 0;
            InsertionCopyState::failAt = failAt;
            EXPECT_THROW(destination.insert(source), std::runtime_error);
            InsertionCopyState::failAt = 0;
            EXPECT_EQ(InsertionCopyState::alive, alive);
            EXPECT_EQ(source.entityCount(), 5);
            EXPECT_EQ(source.componentCount(), 8);
            EXPECT_EQ(destination.entityCount(), 2);
            EXPECT_EQ(destination.componentCount(), 3);
            EXPECT_EQ(destination.get<First>(existing).value, 100);
            EXPECT_EQ(destination.get<Second>(existing).value, 200);
            EXPECT_EQ(destination.get<GE::NameComponent>(named).name, "existing");
            EXPECT_EQ((destination | GE::ECSView<First, Second>()).count(), 1);

            const auto recovered = destination.insert(source);
            EXPECT_EQ(destination.entityCount(), 7);
            for (const auto& [original, copy] : recovered) {
                EXPECT_EQ(destination.get<First>(copy).value, source.get<First>(original).value);
                EXPECT_EQ(destination.has<Second>(copy), source.has<Second>(original));
                if (source.has<Second>(original))
                    EXPECT_EQ(destination.get<Second>(copy).value, source.get<Second>(original).value);
                destination.deleteEntityID(copy);
            }
            EXPECT_EQ(InsertionCopyState::alive, alive);
            destination.remove<Second>(existing);
            destination.emplace<Second>(existing, 200);
        }
    }
    EXPECT_EQ(InsertionCopyState::alive, 0);
}

#ifndef NDEBUG
TEST(ECSRegistryInsertionTest, selfInsertionIsAPreconditionViolation)
{
    GE::ECSRegistry registry;
    EXPECT_DEATH(registry.insert(registry), "source != this");
}

TEST(ECSRegistryInsertionTest, externalReferencesAreAPreconditionViolation)
{
    GE::ECSRegistry source;
    source.emplace<InsertionReference>(source.newEntityID(), uuids::uuid_system_generator{}());
    GE::ECSRegistry destination;
    EXPECT_DEATH(destination.insert(source), "it != ids.end");
}
#endif

}

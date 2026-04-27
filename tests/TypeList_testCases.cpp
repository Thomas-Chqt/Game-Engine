#include <Game-Engine/TypeList.hpp>
#include <Game-Engine/RawInput.hpp>

#include <gtest/gtest.h>

#include <tuple>
#include <type_traits>
#include <vector>

namespace
{

struct FirstType
{
    static constexpr int id = 1;
};

struct SecondType
{
    static constexpr int id = 2;
};

struct ThirdType
{
    static constexpr int id = 3;
};

using TestTypeList = GE::TypeList<FirstType, SecondType, ThirdType>;
using EmptyTypeList = GE::TypeList<>;

template<typename T>
using PointerType = T*;

static_assert(std::is_same_v<TestTypeList::into<std::tuple>, std::tuple<FirstType, SecondType, ThirdType>>);
static_assert(std::is_same_v<TestTypeList::wrapped<PointerType>, GE::TypeList<FirstType*, SecondType*, ThirdType*>>);

static_assert(GE::IsTypeInList<FirstType, TestTypeList>);
static_assert(GE::IsTypeInList<SecondType, TestTypeList>);
static_assert(!GE::IsTypeInList<int, TestTypeList>);
static_assert(GE::IsTypeInList<GE::KeyboardButton, GE::RawInputTypes>);
static_assert(GE::IsTypeInList<GE::MouseButton, GE::RawInputTypes>);
static_assert(!GE::IsTypeInList<int, GE::RawInputTypes>);

static_assert(requires { GE::forEachType<TestTypeList>([]<typename>() {}); });
static_assert(requires { GE::anyOfType<TestTypeList>([]<typename>() { return true; }); });
static_assert(requires { GE::allOfType<TestTypeList>([]<typename>() { return true; }); });
static_assert(requires { GE::noneOfType<TestTypeList>([]<typename>() { return true; }); });

}

TEST(TypeListTest, forEachTypeVisitsTypesInOrder)
{
    std::vector<int> visitedIds;

    GE::forEachType<TestTypeList>([&]<typename T>() {
        visitedIds.push_back(T::id);
    });

    EXPECT_EQ(visitedIds, (std::vector<int>{1, 2, 3}));
}

TEST(TypeListTest, anyOfTypeReturnsTrueWhenAnyTypeMatches)
{
    EXPECT_TRUE((GE::anyOfType<TestTypeList>([]<typename T>() {
        return T::id == 2;
    })));

    EXPECT_FALSE((GE::anyOfType<TestTypeList>([]<typename T>() {
        return T::id == 42;
    })));
}

TEST(TypeListTest, allOfTypeReturnsTrueOnlyWhenAllTypesMatch)
{
    EXPECT_TRUE((GE::allOfType<TestTypeList>([]<typename T>() {
        return T::id > 0;
    })));

    EXPECT_FALSE((GE::allOfType<TestTypeList>([]<typename T>() {
        return T::id < 3;
    })));
}

TEST(TypeListTest, noneOfTypeReturnsTrueOnlyWhenNoTypesMatch)
{
    EXPECT_TRUE((GE::noneOfType<TestTypeList>([]<typename T>() {
        return T::id == 42;
    })));

    EXPECT_FALSE((GE::noneOfType<TestTypeList>([]<typename T>() {
        return T::id == 3;
    })));
}

TEST(TypeListTest, emptyTypeListsUseIdentityFoldValues)
{
    EXPECT_FALSE((GE::anyOfType<EmptyTypeList>([]<typename>() {
        return true;
    })));

    EXPECT_TRUE((GE::allOfType<EmptyTypeList>([]<typename>() {
        return false;
    })));

    EXPECT_TRUE((GE::noneOfType<EmptyTypeList>([]<typename>() {
        return true;
    })));
}

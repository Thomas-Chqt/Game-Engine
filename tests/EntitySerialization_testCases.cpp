#include "../editor/yaml_convert/convert_components.hpp"

#include <gtest/gtest.h>

namespace GE_tests
{

TEST(EntitySerializationTest, uuidRoundTrip)
{
    const auto id = uuids::uuid_system_generator{}();
    const auto encoded = YAML::convert<GE::EntityID>::encode(id);
    EXPECT_EQ(YAML::Load(YAML::Dump(encoded)).as<GE::EntityID>(), id);
}

TEST(EntitySerializationTest, legacyIdsKeepReferencesAndZeroIsValid)
{
    const auto id = YAML::Load("123").as<GE::EntityID>();
    const auto reference = YAML::Load("123").as<std::optional<GE::EntityID>>();
    ASSERT_TRUE(reference.has_value());
    EXPECT_EQ(id, *reference);
    EXPECT_FALSE(YAML::Load("0").as<GE::EntityID>().is_nil());
    EXPECT_NE(id, YAML::Load("124").as<GE::EntityID>());
    const auto encoded = YAML::convert<GE::EntityID>::encode(id);
    EXPECT_EQ(encoded.as<GE::EntityID>(), id);
}

TEST(EntitySerializationTest, absentHierarchyLinksAndLegacySentinel)
{
    EXPECT_FALSE(YAML::Load("null").as<std::optional<GE::EntityID>>().has_value());
    EXPECT_FALSE(YAML::Load("18446744073709551615").as<std::optional<GE::EntityID>>().has_value());
    EXPECT_FALSE(YAML::Load("00000000-0000-0000-0000-000000000000").as<std::optional<GE::EntityID>>().has_value());
    EXPECT_THROW(YAML::Load("18446744073709551615").as<GE::EntityID>(), YAML::BadConversion);
    EXPECT_THROW(YAML::Load("00000000-0000-0000-0000-000000000000").as<GE::EntityID>(), YAML::BadConversion);
    const auto encoded = YAML::convert<std::optional<GE::EntityID>>::encode(std::nullopt);
    EXPECT_TRUE(encoded.IsNull());
    EXPECT_FALSE(encoded.as<std::optional<GE::EntityID>>().has_value());
    EXPECT_THROW(YAML::Load("invalid-id").as<GE::EntityID>(), YAML::BadConversion);
}

}

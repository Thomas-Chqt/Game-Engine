/*
 * ---------------------------------------------------
 * serialization_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/Components.hpp"
#include "Game-Engine/AssetManager.hpp"

#include <Graphics/Texture.hpp>

#include <yaml-cpp/yaml.h>

#include <map>
#include <vector>

namespace GE_tests
{

TEST(SerializationTest, componentVariantMapYamlRoundTrip)
{
    std::map<GE::ECSWorld::EntityID, std::vector<GE::ComponentVariant>> entities = {
        {
            2, {
                GE::NameComponent{"camera"},
                GE::TransformComponent{
                    .position = {3.0f, 4.0f, 5.0f},
                    .rotation = {0.1f, 0.2f, 0.3f},
                    .scale = {1.0f, 2.0f, 3.0f}
                },
                GE::CameraComponent{
                    .fov = 1.2f,
                    .zFar = 500.0f,
                    .zNear = 0.5f
                }
            }
        },
        {
            7, {
                GE::HierarchyComponent{
                    .parent = 2,
                    .firstChild = INVALID_ENTITY_ID,
                    .nextChild = INVALID_ENTITY_ID
                },
                GE::LightComponent{
                    .type = GE::LightComponent::Type::directional,
                    .color = {0.5f, 0.6f, 0.7f},
                    .intentsity = 0.8f,
                    .attenuation = 0.9f
                },
                GE::MeshComponent{GE::BUILT_IN_CUBE_ASSET_ID}
            }
        }
    };

    YAML::Node node = YAML::convert<decltype(entities)>::encode(entities);
    const std::string yaml = YAML::Dump(node);

    EXPECT_NE(yaml.find("NameComponent"), std::string::npos);
    EXPECT_NE(yaml.find("LightComponent"), std::string::npos);
    EXPECT_NE(yaml.find("MeshComponent"), std::string::npos);

    auto decoded = node.as<decltype(entities)>();

    ASSERT_EQ(decoded.size(), entities.size());
    ASSERT_TRUE(decoded.contains(2));
    ASSERT_TRUE(decoded.contains(7));

    ASSERT_EQ(decoded.at(2).size(), 3u);
    EXPECT_EQ(std::get<GE::NameComponent>(decoded.at(2)[0]).name, "camera");
    const auto& transform = std::get<GE::TransformComponent>(decoded.at(2)[1]);
    EXPECT_FLOAT_EQ(transform.position.x, 3.0f);
    EXPECT_FLOAT_EQ(transform.position.y, 4.0f);
    EXPECT_FLOAT_EQ(transform.position.z, 5.0f);
    EXPECT_FLOAT_EQ(std::get<GE::CameraComponent>(decoded.at(2)[2]).zNear, 0.5f);

    ASSERT_EQ(decoded.at(7).size(), 3u);
    EXPECT_EQ(std::get<GE::HierarchyComponent>(decoded.at(7)[0]).parent, 2u);
    EXPECT_EQ(std::get<GE::LightComponent>(decoded.at(7)[1]).type, GE::LightComponent::Type::directional);
    EXPECT_EQ(std::get<GE::MeshComponent>(decoded.at(7)[2]).id, GE::BUILT_IN_CUBE_ASSET_ID);
}

TEST(AssetPathYamlConversion, encodesMeshAssetPathWithTypeAndPath)
{
    const std::filesystem::path meshPath("meshes/cube.obj");
    const GE::VAssetPath assetPath = GE::AssetPath<GE::Mesh>(meshPath);

    const YAML::Node node = YAML::convert<GE::VAssetPath>::encode(assetPath);

    ASSERT_TRUE(node.IsMap());
    EXPECT_EQ(node["type"].as<std::string>(), "Mesh");
    EXPECT_EQ(node["path"].as<std::string>(), meshPath.string());
}

TEST(AssetPathYamlConversion, decodesMeshAssetPathFromYaml)
{
    YAML::Node node;
    node["type"] = "Mesh";
    node["path"] = "meshes/cube.obj";

    GE::VAssetPath assetPath;
    ASSERT_TRUE(YAML::convert<GE::VAssetPath>::decode(node, assetPath));

    ASSERT_TRUE(std::holds_alternative<GE::AssetPath<GE::Mesh>>(assetPath));
    EXPECT_EQ(std::get<GE::AssetPath<GE::Mesh>>(assetPath).path, std::filesystem::path("meshes/cube.obj"));
}

TEST(AssetPathYamlConversion, roundTripsTextureAssetPathThroughYamlNode)
{
    const GE::VAssetPath assetPath = GE::AssetPath<gfx::Texture>(std::filesystem::path("textures/dummy.png"));

    const YAML::Node encoded = YAML::convert<GE::VAssetPath>::encode(assetPath);
    const GE::VAssetPath decoded = encoded.as<GE::VAssetPath>();

    ASSERT_TRUE(std::holds_alternative<GE::AssetPath<gfx::Texture>>(decoded));
    EXPECT_EQ(std::get<GE::AssetPath<gfx::Texture>>(decoded).path, std::filesystem::path("textures/dummy.png"));
}

TEST(AssetPathYamlConversion, rejectsYamlWithoutRequiredKeys)
{
    YAML::Node missingPath;
    missingPath["type"] = "Mesh";

    YAML::Node missingType;
    missingType["path"] = "meshes/cube.obj";

    GE::VAssetPath assetPath;
    EXPECT_FALSE(YAML::convert<GE::VAssetPath>::decode(missingPath, assetPath));
    EXPECT_FALSE(YAML::convert<GE::VAssetPath>::decode(missingType, assetPath));
}

TEST(AssetPathYamlConversion, rejectsUnknownAssetType)
{
    YAML::Node node;
    node["type"] = "Material";
    node["path"] = "materials/default.mat";

    GE::VAssetPath assetPath;
    EXPECT_FALSE(YAML::convert<GE::VAssetPath>::decode(node, assetPath));
}

} // namespace GE_tests

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
#include "Game-Engine/Scene.hpp"

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

TEST(SceneDescriptorYamlConversion, encodesDescriptorFields)
{
    const GE::Scene::Descriptor descriptor{
        .name = "MainScene",
        .activeCamera = 42,
        .registredAssets = {
            {3, GE::AssetPath<GE::Mesh>(std::filesystem::path("meshes/cube.obj"))},
            {5, GE::AssetPath<gfx::Texture>(std::filesystem::path("textures/albedo.png"))}
        },
        .entities = {
            {42, {
                GE::NameComponent{"camera"},
                GE::TransformComponent{
                    .position = {1.0f, 2.0f, 3.0f},
                    .rotation = {0.1f, 0.2f, 0.3f},
                    .scale = {4.0f, 5.0f, 6.0f}
                },
                GE::CameraComponent{
                    .fov = 1.4f,
                    .zFar = 600.0f,
                    .zNear = 0.2f
                }
            }},
            {9, {
                GE::HierarchyComponent{
                    .parent = 42,
                    .firstChild = INVALID_ENTITY_ID,
                    .nextChild = INVALID_ENTITY_ID
                },
                GE::MeshComponent{3}
            }}
        }
    };

    const YAML::Node node = YAML::convert<GE::Scene::Descriptor>::encode(descriptor);

    ASSERT_TRUE(node.IsMap());
    EXPECT_EQ(node["name"].as<std::string>(), "MainScene");
    EXPECT_EQ(node["activeCamera"].as<GE::ECSWorld::EntityID>(), 42u);
    ASSERT_TRUE(node["registredAssets"].IsMap());
    ASSERT_TRUE(node["entities"].IsMap());
    EXPECT_EQ(node["registredAssets"]["3"]["type"].as<std::string>(), "Mesh");
    EXPECT_EQ(node["registredAssets"]["3"]["path"].as<std::string>(), "meshes/cube.obj");
    EXPECT_EQ(node["entities"]["42"][0]["type"].as<std::string>(), "NameComponent");
    EXPECT_EQ(node["entities"]["42"][0]["data"]["name"].as<std::string>(), "camera");
}

TEST(SceneDescriptorYamlConversion, decodesDescriptorFromYaml)
{
    YAML::Node node;
    node["name"] = "LoadedScene";
    node["activeCamera"] = 7;
    node["registredAssets"]["11"]["type"] = "Texture";
    node["registredAssets"]["11"]["path"] = "textures/diffuse.png";
    node["entities"]["7"].push_back(YAML::Load("{ type: NameComponent, data: { name: player } }"));
    node["entities"]["7"].push_back(YAML::Load(
        "{ type: TransformComponent, data: { position: [7.0, 8.0, 9.0], rotation: [0.0, 0.5, 1.0], scale: [1.0, 1.5, 2.0] } }"
    ));
    node["entities"]["7"].push_back(YAML::Load(
        "{ type: CameraComponent, data: { fov: 1.1, zFar: 250.0, zNear: 0.3 } }"
    ));

    GE::Scene::Descriptor descriptor;
    ASSERT_TRUE(YAML::convert<GE::Scene::Descriptor>::decode(node, descriptor));

    EXPECT_EQ(descriptor.name, "LoadedScene");
    EXPECT_EQ(descriptor.activeCamera, 7u);
    ASSERT_EQ(descriptor.registredAssets.size(), 1u);
    ASSERT_TRUE(descriptor.registredAssets.contains(11));
    ASSERT_TRUE(std::holds_alternative<GE::AssetPath<gfx::Texture>>(descriptor.registredAssets.at(11)));
    EXPECT_EQ(
        std::get<GE::AssetPath<gfx::Texture>>(descriptor.registredAssets.at(11)).path,
        std::filesystem::path("textures/diffuse.png")
    );

    ASSERT_EQ(descriptor.entities.size(), 1u);
    ASSERT_TRUE(descriptor.entities.contains(7));
    ASSERT_EQ(descriptor.entities.at(7).size(), 3u);
    EXPECT_EQ(std::get<GE::NameComponent>(descriptor.entities.at(7)[0]).name, "player");
    const auto& transform = std::get<GE::TransformComponent>(descriptor.entities.at(7)[1]);
    EXPECT_FLOAT_EQ(transform.position.x, 7.0f);
    EXPECT_FLOAT_EQ(transform.rotation.y, 0.5f);
    EXPECT_FLOAT_EQ(transform.scale.z, 2.0f);
    EXPECT_FLOAT_EQ(std::get<GE::CameraComponent>(descriptor.entities.at(7)[2]).zNear, 0.3f);
}

TEST(SceneDescriptorYamlConversion, roundTripsDescriptorThroughYamlNode)
{
    const GE::Scene::Descriptor descriptor{
        .name = "RoundTripScene",
        .activeCamera = 15,
        .registredAssets = {
            {2, GE::AssetPath<GE::Mesh>(std::filesystem::path("meshes/player.obj"))}
        },
        .entities = {
            {15, {
                GE::NameComponent{"player"},
                GE::MeshComponent{2}
            }}
        }
    };

    const YAML::Node encoded = YAML::convert<GE::Scene::Descriptor>::encode(descriptor);
    const GE::Scene::Descriptor decoded = encoded.as<GE::Scene::Descriptor>();

    EXPECT_EQ(decoded.name, descriptor.name);
    EXPECT_EQ(decoded.activeCamera, descriptor.activeCamera);
    ASSERT_EQ(decoded.registredAssets.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<GE::AssetPath<GE::Mesh>>(decoded.registredAssets.at(2)));
    EXPECT_EQ(
        std::get<GE::AssetPath<GE::Mesh>>(decoded.registredAssets.at(2)).path,
        std::filesystem::path("meshes/player.obj")
    );
    ASSERT_EQ(decoded.entities.size(), 1u);
    ASSERT_EQ(decoded.entities.at(15).size(), 2u);
    EXPECT_EQ(std::get<GE::NameComponent>(decoded.entities.at(15)[0]).name, "player");
    EXPECT_EQ(std::get<GE::MeshComponent>(decoded.entities.at(15)[1]).id, 2u);
}

TEST(SceneDescriptorYamlConversion, rejectsYamlWithoutRequiredKeys)
{
    YAML::Node missingName;
    missingName["activeCamera"] = 1;
    missingName["registredAssets"] = YAML::Node(YAML::NodeType::Map);
    missingName["entities"] = YAML::Node(YAML::NodeType::Map);

    YAML::Node missingActiveCamera;
    missingActiveCamera["name"] = "Scene";
    missingActiveCamera["registredAssets"] = YAML::Node(YAML::NodeType::Map);
    missingActiveCamera["entities"] = YAML::Node(YAML::NodeType::Map);

    YAML::Node missingRegistredAssets;
    missingRegistredAssets["name"] = "Scene";
    missingRegistredAssets["activeCamera"] = 1;
    missingRegistredAssets["entities"] = YAML::Node(YAML::NodeType::Map);

    GE::Scene::Descriptor descriptor;
    EXPECT_FALSE(YAML::convert<GE::Scene::Descriptor>::decode(missingName, descriptor));
    EXPECT_FALSE(YAML::convert<GE::Scene::Descriptor>::decode(missingActiveCamera, descriptor));
    EXPECT_FALSE(YAML::convert<GE::Scene::Descriptor>::decode(missingRegistredAssets, descriptor));
}

} // namespace GE_tests

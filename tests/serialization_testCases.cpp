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
#include "Game-Engine/InputContext.hpp"
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
                GE::MeshComponent{0}
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
    EXPECT_EQ(std::get<GE::MeshComponent>(decoded.at(7)[2]).id, 0);
}

TEST(RegisteredAssetYamlConversion, encodesAssetPathAndId)
{
    const std::pair<std::optional<GE::VAssetLocation>, GE::AssetID> registeredAsset = {
        GE::AssetLocation<GE::Mesh>{std::filesystem::path("meshes/cube.obj"), 0},
        3
    };

    const YAML::Node node = YAML::convert<std::pair<std::optional<GE::VAssetLocation>, GE::AssetID>>::encode(registeredAsset);

    ASSERT_TRUE(node.IsMap());
    EXPECT_EQ(node["assetId"].as<GE::AssetID>(), 3u);
    ASSERT_TRUE(node["asset"]);
    EXPECT_EQ(node["asset"]["type"].as<std::string>(), "Mesh");
    EXPECT_EQ(node["asset"]["containerPath"].as<std::string>(), "meshes/cube.obj");
    EXPECT_EQ(node["asset"]["index"].as<uint32_t>(), 0u);
}

TEST(RegisteredAssetYamlConversion, decodesAssetPathAndIdFromYaml)
{
    YAML::Node node;
    node["assetId"] = 3;
    node["asset"]["type"] = "Mesh";
    node["asset"]["containerPath"] = "meshes/cube.obj";
    node["asset"]["index"] = 0;

    std::pair<std::optional<GE::VAssetLocation>, GE::AssetID> registeredAsset{ std::nullopt, 0 };
    ASSERT_TRUE(YAML::convert<decltype(registeredAsset)>::decode(node, registeredAsset));

    EXPECT_EQ(registeredAsset.second, 3u);
    ASSERT_TRUE(registeredAsset.first.has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<GE::Mesh>>(*registeredAsset.first));
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(*registeredAsset.first).containerPath, std::filesystem::path("meshes/cube.obj"));
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(*registeredAsset.first).index, 0u);
}

TEST(RegisteredAssetYamlConversion, roundTripsTextureAssetPathThroughYamlNode)
{
    const std::pair<std::optional<GE::VAssetLocation>, GE::AssetID> registeredAsset = {
        GE::AssetLocation<gfx::Texture>{std::filesystem::path("textures/dummy.png"), 4},
        9
    };

    const YAML::Node encoded = YAML::convert<std::pair<std::optional<GE::VAssetLocation>, GE::AssetID>>::encode(registeredAsset);
    const auto decoded = encoded.as<std::pair<std::optional<GE::VAssetLocation>, GE::AssetID>>();

    EXPECT_EQ(decoded.second, 9u);
    ASSERT_TRUE(decoded.first.has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<gfx::Texture>>(*decoded.first));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*decoded.first).containerPath, std::filesystem::path("textures/dummy.png"));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*decoded.first).index, 4u);
}

TEST(RegisteredAssetYamlConversion, acceptsMissingAssetPathForExistingAssetId)
{
    YAML::Node node;
    node["assetId"] = 11;

    std::pair<std::optional<GE::VAssetLocation>, GE::AssetID> registeredAsset{ GE::AssetLocation<GE::Mesh>{std::filesystem::path("fallback.obj"), 8}, 0 };
    ASSERT_TRUE(YAML::convert<decltype(registeredAsset)>::decode(node, registeredAsset));

    EXPECT_EQ(registeredAsset.second, 11u);
    EXPECT_FALSE(registeredAsset.first.has_value());
}

TEST(RegisteredAssetYamlConversion, rejectsYamlWithoutRequiredKeys)
{
    YAML::Node missingAssetId;
    missingAssetId["asset"]["type"] = "Mesh";
    missingAssetId["asset"]["containerPath"] = "meshes/cube.obj";
    missingAssetId["asset"]["index"] = 0;

    YAML::Node missingPath;
    missingPath["assetId"] = 3;
    missingPath["asset"]["type"] = "Mesh";

    YAML::Node missingType;
    missingType["assetId"] = 3;
    missingType["asset"]["containerPath"] = "meshes/cube.obj";
    missingType["asset"]["index"] = 0;

    std::pair<std::optional<GE::VAssetLocation>, GE::AssetID> registeredAsset{ std::nullopt, 0 };
    EXPECT_FALSE(YAML::convert<decltype(registeredAsset)>::decode(missingAssetId, registeredAsset));
    EXPECT_FALSE(YAML::convert<decltype(registeredAsset)>::decode(missingPath, registeredAsset));
    EXPECT_FALSE(YAML::convert<decltype(registeredAsset)>::decode(missingType, registeredAsset));
}

TEST(RegisteredAssetYamlConversion, rejectsUnknownAssetType)
{
    YAML::Node node;
    node["assetId"] = 3;
    node["asset"]["type"] = "Material";
    node["asset"]["containerPath"] = "materials/default.mat";
    node["asset"]["index"] = 0;

    std::pair<std::optional<GE::VAssetLocation>, GE::AssetID> registeredAsset{ std::nullopt, 0 };
    EXPECT_FALSE(YAML::convert<decltype(registeredAsset)>::decode(node, registeredAsset));
}

TEST(SceneDescriptorYamlConversion, encodesDescriptorFields)
{
    const GE::Scene::Descriptor descriptor{
        .name = "MainScene",
        .activeCamera = 42,
        .registredAssets = {
            {GE::AssetLocation<GE::Mesh>{std::filesystem::path("meshes/cube.obj"), 0}, 3},
            {std::nullopt, 4},
            {GE::AssetLocation<gfx::Texture>{std::filesystem::path("textures/albedo.png"), 2}, 5}
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
    ASSERT_TRUE(node["registredAssets"].IsSequence());
    ASSERT_TRUE(node["entities"].IsSequence());
    ASSERT_EQ(node["registredAssets"].size(), 3u);
    EXPECT_EQ(node["registredAssets"][0]["assetId"].as<GE::AssetID>(), 3u);
    EXPECT_EQ(node["registredAssets"][0]["asset"]["type"].as<std::string>(), "Mesh");
    EXPECT_EQ(node["registredAssets"][0]["asset"]["containerPath"].as<std::string>(), "meshes/cube.obj");
    EXPECT_EQ(node["registredAssets"][0]["asset"]["index"].as<uint32_t>(), 0u);
    EXPECT_EQ(node["registredAssets"][1]["assetId"].as<GE::AssetID>(), 4u);
    EXPECT_FALSE(node["registredAssets"][1]["asset"]);
    EXPECT_EQ(node["entities"][0]["entityId"].as<GE::ECSWorld::EntityID>(), 42u);
    EXPECT_EQ(node["entities"][0]["components"][0]["type"].as<std::string>(), "NameComponent");
    EXPECT_EQ(node["entities"][0]["components"][0]["data"]["name"].as<std::string>(), "camera");
}

TEST(SceneDescriptorYamlConversion, decodesDescriptorFromYaml)
{
    YAML::Node node;
    node["name"] = "LoadedScene";
    node["activeCamera"] = 7;
    node["registredAssets"].push_back(YAML::Load("{ assetId: 11, asset: { type: Texture, containerPath: textures/diffuse.png, index: 5 } }"));
    node["registredAssets"].push_back(YAML::Load("{ assetId: 12 }"));
    node["entities"].push_back(YAML::Load(
        "{ entityId: 7, components: ["
        "{ type: NameComponent, data: { name: player } },"
        "{ type: TransformComponent, data: { position: [7.0, 8.0, 9.0], rotation: [0.0, 0.5, 1.0], scale: [1.0, 1.5, 2.0] } },"
        "{ type: CameraComponent, data: { fov: 1.1, zFar: 250.0, zNear: 0.3 } }"
        "] }"
    ));

    GE::Scene::Descriptor descriptor;
    ASSERT_TRUE(YAML::convert<GE::Scene::Descriptor>::decode(node, descriptor));

    EXPECT_EQ(descriptor.name, "LoadedScene");
    EXPECT_EQ(descriptor.activeCamera, 7u);
    ASSERT_EQ(descriptor.registredAssets.size(), 2u);
    EXPECT_EQ(descriptor.registredAssets[0].second, 11u);
    ASSERT_TRUE(descriptor.registredAssets[0].first.has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<gfx::Texture>>(*descriptor.registredAssets[0].first));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*descriptor.registredAssets[0].first).containerPath, std::filesystem::path("textures/diffuse.png"));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*descriptor.registredAssets[0].first).index, 5u);
    EXPECT_EQ(descriptor.registredAssets[1].second, 12u);
    EXPECT_FALSE(descriptor.registredAssets[1].first.has_value());

    ASSERT_EQ(descriptor.entities.size(), 1u);
    EXPECT_EQ(descriptor.entities[0].first, 7u);
    ASSERT_EQ(descriptor.entities[0].second.size(), 3u);
    EXPECT_EQ(std::get<GE::NameComponent>(descriptor.entities[0].second[0]).name, "player");
    const auto& transform = std::get<GE::TransformComponent>(descriptor.entities[0].second[1]);
    EXPECT_FLOAT_EQ(transform.position.x, 7.0f);
    EXPECT_FLOAT_EQ(transform.rotation.y, 0.5f);
    EXPECT_FLOAT_EQ(transform.scale.z, 2.0f);
    EXPECT_FLOAT_EQ(std::get<GE::CameraComponent>(descriptor.entities[0].second[2]).zNear, 0.3f);
}

TEST(SceneDescriptorYamlConversion, roundTripsDescriptorThroughYamlNode)
{
    const GE::Scene::Descriptor descriptor{
        .name = "RoundTripScene",
        .activeCamera = 15,
        .registredAssets = {
            {GE::AssetLocation<GE::Mesh>{std::filesystem::path("meshes/player.obj"), 0}, 2},
            {std::nullopt, 3}
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
    ASSERT_EQ(decoded.registredAssets.size(), 2u);
    EXPECT_EQ(decoded.registredAssets[0].second, 2u);
    ASSERT_TRUE(decoded.registredAssets[0].first.has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<GE::Mesh>>(*decoded.registredAssets[0].first));
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(*decoded.registredAssets[0].first).containerPath, std::filesystem::path("meshes/player.obj"));
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(*decoded.registredAssets[0].first).index, 0u);
    EXPECT_EQ(decoded.registredAssets[1].second, 3u);
    EXPECT_FALSE(decoded.registredAssets[1].first.has_value());
    ASSERT_EQ(decoded.entities.size(), 1u);
    EXPECT_EQ(decoded.entities[0].first, 15u);
    ASSERT_EQ(decoded.entities[0].second.size(), 2u);
    EXPECT_EQ(std::get<GE::NameComponent>(decoded.entities[0].second[0]).name, "player");
    EXPECT_EQ(std::get<GE::MeshComponent>(decoded.entities[0].second[1]).id, 2u);
}

TEST(SceneDescriptorYamlConversion, rejectsYamlWithoutRequiredKeys)
{
    YAML::Node missingName;
    missingName["activeCamera"] = 1;
    missingName["registredAssets"] = YAML::Node(YAML::NodeType::Sequence);
    missingName["entities"] = YAML::Node(YAML::NodeType::Sequence);

    YAML::Node missingActiveCamera;
    missingActiveCamera["name"] = "Scene";
    missingActiveCamera["registredAssets"] = YAML::Node(YAML::NodeType::Sequence);
    missingActiveCamera["entities"] = YAML::Node(YAML::NodeType::Sequence);

    YAML::Node missingRegistredAssets;
    missingRegistredAssets["name"] = "Scene";
    missingRegistredAssets["activeCamera"] = 1;
    missingRegistredAssets["entities"] = YAML::Node(YAML::NodeType::Sequence);

    GE::Scene::Descriptor descriptor;
    EXPECT_FALSE(YAML::convert<GE::Scene::Descriptor>::decode(missingName, descriptor));
    EXPECT_TRUE(YAML::convert<GE::Scene::Descriptor>::decode(missingActiveCamera, descriptor));
    EXPECT_EQ(descriptor.activeCamera, INVALID_ENTITY_ID);
    EXPECT_TRUE(descriptor.registredAssets.empty());
    EXPECT_TRUE(descriptor.entities.empty());

    EXPECT_TRUE(YAML::convert<GE::Scene::Descriptor>::decode(missingRegistredAssets, descriptor));
    EXPECT_EQ(descriptor.activeCamera, 1u);
    EXPECT_TRUE(descriptor.registredAssets.empty());
    EXPECT_TRUE(descriptor.entities.empty());
}

TEST(InputContextYamlConversion, roundTripsConfiguredInputsThroughYamlNode)
{
    GE::InputContext inputContext;

    GE::ActionInput jumpInput;
    jumpInput.setMapper<GE::KeyboardButton>(GE::KeyboardButton::space);
    inputContext.addInput("jump", jumpInput);

    GE::StateInput brakeInput;
    brakeInput.setMapper<GE::KeyboardButton>(GE::KeyboardButton::s);
    inputContext.addInput("brake", brakeInput);

    GE::RangeInput throttleInput;
    throttleInput.setMapper<GE::KeyboardButton>(GE::KeyboardButton::w, 2.5f);
    inputContext.addInput("throttle", throttleInput);

    GE::Range2DInput moveInput;
    moveInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::d,
        .xNeg = GE::KeyboardButton::a,
        .xScale = 2.0f,
        .yPos = GE::KeyboardButton::w,
        .yNeg = GE::KeyboardButton::s,
        .yScale = 3.0f,
        .triggerValue = 0.2f
    });
    inputContext.addInput("move", moveInput);

    const YAML::Node encoded = YAML::convert<GE::InputContext>::encode(inputContext);
    EXPECT_EQ(encoded["inputs"]["jump"]["type"].as<std::string>(), "Action");
    EXPECT_EQ(encoded["inputs"]["jump"]["mapper"]["type"].as<std::string>(), "KeyboardButton");
    EXPECT_EQ(encoded["inputs"]["brake"]["type"].as<std::string>(), "State");
    const GE::InputContext decoded = encoded.as<GE::InputContext>();

    ASSERT_EQ(decoded.inputs().size(), 4u);
    ASSERT_TRUE(decoded.inputs().contains("jump"));
    ASSERT_TRUE(decoded.inputs().contains("brake"));
    ASSERT_TRUE(decoded.inputs().contains("throttle"));
    ASSERT_TRUE(decoded.inputs().contains("move"));

    const auto& jump = std::get<GE::ActionInput>(decoded.inputs().at("jump"));
    ASSERT_TRUE(jump.mapper.has_value());
    const auto& jumpMapper = std::get<GE::InputMapper<GE::KeyboardButton, GE::ActionInput>>(*jump.mapper);
    EXPECT_EQ(jumpMapper.button, GE::KeyboardButton::space);

    const auto& brake = std::get<GE::StateInput>(decoded.inputs().at("brake"));
    ASSERT_TRUE(brake.mapper.has_value());
    const auto& brakeMapper = std::get<GE::InputMapper<GE::KeyboardButton, GE::StateInput>>(*brake.mapper);
    EXPECT_EQ(brakeMapper.button, GE::KeyboardButton::s);

    const auto& throttle = std::get<GE::RangeInput>(decoded.inputs().at("throttle"));
    ASSERT_TRUE(throttle.mapper.has_value());
    const auto& throttleMapper = std::get<GE::InputMapper<GE::KeyboardButton, GE::RangeInput>>(*throttle.mapper);
    EXPECT_EQ(throttleMapper.button, GE::KeyboardButton::w);
    EXPECT_FLOAT_EQ(throttleMapper.scale, 2.5f);

    const auto& move = std::get<GE::Range2DInput>(decoded.inputs().at("move"));
    ASSERT_TRUE(move.mapper.has_value());
    const auto& moveMapper = std::get<GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>>(*move.mapper);
    EXPECT_EQ(moveMapper.xPos, GE::KeyboardButton::d);
    EXPECT_EQ(moveMapper.xNeg, GE::KeyboardButton::a);
    EXPECT_EQ(moveMapper.yPos, GE::KeyboardButton::w);
    EXPECT_EQ(moveMapper.yNeg, GE::KeyboardButton::s);
    EXPECT_FLOAT_EQ(moveMapper.xScale, 2.0f);
    EXPECT_FLOAT_EQ(moveMapper.yScale, 3.0f);
    EXPECT_FLOAT_EQ(moveMapper.triggerValue, 0.2f);
}

TEST(InputDispatch, emptyCallbackDoesNotThrowWhenInputIsTriggered)
{
    GE::ActionInput input;
    input.triggered = true;

    EXPECT_NO_THROW(input.dispatch());
    EXPECT_FALSE(input.triggered);
}

} // namespace GE_tests

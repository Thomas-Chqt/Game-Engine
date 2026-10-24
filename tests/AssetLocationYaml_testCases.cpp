#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include "../editor/yaml_convert/convert_asset.hpp"

namespace GE_tests
{
namespace
{
using SerializedAsset = std::tuple<std::string, GE::VAssetLocation, GE::AssetID, std::vector<GE::AssetID>>;
}

TEST(AssetLocationYamlTest, textureFormatRoundTrips)
{
    for (const auto format : {gfx::PixelFormat::RGBA8_sRGB, gfx::PixelFormat::RGBA8_unorm}) {
        const GE::AssetLocation<gfx::Texture> location{"material.gltf", 3, format};
        const SerializedAsset original{"texture", GE::VAssetLocation(location), 42, {}};
        const YAML::Node encoded = YAML::convert<SerializedAsset>::encode(original);
        const auto decoded = YAML::Load(YAML::Dump(encoded)).as<SerializedAsset>();
        EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(std::get<1>(decoded)), location);
        EXPECT_EQ(std::get<2>(decoded), 42u);
    }
}

TEST(AssetLocationYamlTest, oldTextureLocationDefaultsToSrgb)
{
    const auto decoded = YAML::Load(R"(
name: texture
id: 2
location:
  type: Texture
  data:
    containerPath: material.gltf
    index: 3
)").as<SerializedAsset>();
    const auto& location = std::get<GE::AssetLocation<gfx::Texture>>(std::get<1>(decoded));
    EXPECT_EQ(location.pixelFormat, gfx::PixelFormat::RGBA8_sRGB);
    EXPECT_EQ(location.index, 3u);
}

TEST(AssetLocationYamlTest, meshLocationRemainsUnchanged)
{
    const GE::AssetLocation<GE::Mesh> location{"mesh.gltf", 2};
    const SerializedAsset original{"mesh", GE::VAssetLocation(location), 5, {1}};
    const auto encoded = YAML::convert<SerializedAsset>::encode(original);
    EXPECT_FALSE(encoded["location"]["data"]["pixelFormat"]);
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(std::get<1>(encoded.as<SerializedAsset>())), location);
}
}

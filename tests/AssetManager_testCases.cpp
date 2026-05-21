/*
 * ---------------------------------------------------
 * AssetManager_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/AssetManagerView.hpp"
#include "Game-Engine/MeshAssetLoader.hpp"
#include "Game-Engine/Scene.hpp"
#include "Game-Engine/TextureAssetLoader.hpp"

#include <Graphics/Buffer.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/CommandBufferPool.hpp>
#include <Graphics/Device.hpp>
#include <Graphics/Texture.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace GE_tests
{

namespace
{

class MockBuffer final : public gfx::Buffer
{
public:
    explicit MockBuffer(const Descriptor& desc)
        : m_desc(desc)
        , m_bytes(desc.size)
    {
    }

    size_t size() const override { return m_desc.size; }
    gfx::BufferUsages usages() const override { return m_desc.usages; }
    gfx::ResourceStorageMode storageMode() const override { return m_desc.storageMode; }

    void setContent(const void* data, size_t size) override
    {
        if (size > m_bytes.size())
            throw std::runtime_error("buffer overflow in mock buffer");
        std::memcpy(m_bytes.data(), data, size);
    }

protected:
    void* contentVoid() override { return m_bytes.data(); }

private:
    Descriptor m_desc;
    std::vector<std::byte> m_bytes;
};

class TestTexture final : public gfx::Texture
{
public:
    explicit TestTexture(const Descriptor& desc)
        : m_desc(desc)
    {
    }

    gfx::TextureType type() const override { return m_desc.type; }
    uint32_t width() const override { return m_desc.width; }
    uint32_t height() const override { return m_desc.height; }
    gfx::PixelFormat pixelFormat() const override { return m_desc.pixelFormat; }
    gfx::TextureUsages usages() const override { return m_desc.usages; }
    gfx::ResourceStorageMode storageMode() const override { return m_desc.storageMode; }

#if defined(GFX_IMGUI_ENABLED)
    void initImTextureId() override {}
    std::optional<uint64_t> imTextureId() const override { return std::nullopt; }
#endif

private:
    Descriptor m_desc;
};

class MockCommandBuffer : public gfx::CommandBuffer
{
public:
    MOCK_METHOD(void, beginRenderPass, (const gfx::Framebuffer&), (override));
    MOCK_METHOD(void, usePipeline, ((const std::shared_ptr<const gfx::GraphicsPipeline>&)), (override));
    MOCK_METHOD(void, useVertexBuffer, ((const std::shared_ptr<gfx::Buffer>&)), (override));
    MOCK_METHOD(void, setParameterBlock, ((const std::shared_ptr<const gfx::ParameterBlock>&), uint32_t), (override));
    MOCK_METHOD(void, setPushConstants, (const void*, size_t), (override));
    MOCK_METHOD(void, drawVertices, (uint32_t, uint32_t), (override));
    MOCK_METHOD(void, drawIndexedVertices, ((const std::shared_ptr<gfx::Buffer>&)), (override));
#if defined(GFX_IMGUI_ENABLED)
    MOCK_METHOD(void, imGuiRenderDrawData, (ImDrawData*), (const, override));
#endif
    MOCK_METHOD(void, endRenderPass, (), (override));

    MOCK_METHOD(void, beginBlitPass, (), (override));
    MOCK_METHOD(void, copyBufferToBuffer, ((const std::shared_ptr<gfx::Buffer>&), (const std::shared_ptr<gfx::Buffer>&), size_t), (override));
    MOCK_METHOD(void, copyBufferToTexture, ((const std::shared_ptr<gfx::Buffer>&), size_t, (const std::shared_ptr<gfx::Texture>&), uint32_t), (override));
    MOCK_METHOD(void, endBlitPass, (), (override));
    MOCK_METHOD(void, presentDrawable, ((const std::shared_ptr<gfx::Drawable>&)), (override));
    MOCK_METHOD(void, addSampledTexture, ((const std::shared_ptr<gfx::Texture>&)), (override));
};

class MockCommandBufferPool : public gfx::CommandBufferPool
{
public:
    MOCK_METHOD(std::shared_ptr<gfx::CommandBuffer>, get, (), (override));
    MOCK_METHOD(void, reset, (), (override));
};

class MockDevice : public gfx::Device
{
public:
    MOCK_METHOD(gfx::Backend, backend, (), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::Swapchain>, newSwapchain, (const gfx::Swapchain::Descriptor&), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::ShaderLib>, newShaderLib, (const std::filesystem::path&), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::ParameterBlockLayout>, newParameterBlockLayout, (const gfx::ParameterBlockLayout::Descriptor&), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::GraphicsPipeline>, newGraphicsPipeline, (const gfx::GraphicsPipeline::Descriptor&), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::Buffer>, newBuffer, (const gfx::Buffer::Descriptor&), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::Texture>, newTexture, (const gfx::Texture::Descriptor&), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::CommandBufferPool>, newCommandBufferPool, (), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::ParameterBlockPool>, newParameterBlockPool, (const gfx::ParameterBlockPool::Descriptor&), (const, override));
    MOCK_METHOD(std::unique_ptr<gfx::Sampler>, newSampler, (const gfx::Sampler::Descriptor&), (const, override));

#if defined(GFX_IMGUI_ENABLED)
    MOCK_METHOD(void, imguiInit, (std::vector<gfx::PixelFormat>, std::optional<gfx::PixelFormat>), (const, override));
    MOCK_METHOD(void, imguiNewFrame, (), (const, override));
    MOCK_METHOD(void, imguiShutdown, (), (override));
#endif

    MOCK_METHOD(void, submitCommandBuffers, ((const std::shared_ptr<gfx::CommandBuffer>&)), (override));
    MOCK_METHOD(void, submitCommandBuffers, ((const std::vector<std::shared_ptr<gfx::CommandBuffer>>&)), (override));
    MOCK_METHOD(void, waitCommandBuffer, (const gfx::CommandBuffer&), (override));
    MOCK_METHOD(void, waitIdle, (), (override));
};

class AssetManagerMockDeviceTest : public ::testing::Test
{
protected:
    AssetManagerMockDeviceTest()
        : m_commandBuffer(std::make_shared<testing::NiceMock<MockCommandBuffer>>())
    {
        ON_CALL(m_device, backend()).WillByDefault(testing::Return(gfx::Backend::metal));
        ON_CALL(m_device, newBuffer(testing::_)).WillByDefault([](const gfx::Buffer::Descriptor& desc) {
            return std::make_unique<MockBuffer>(desc);
        });
        ON_CALL(m_device, newTexture(testing::_)).WillByDefault([](const gfx::Texture::Descriptor& desc) {
            return std::make_unique<TestTexture>(desc);
        });
        ON_CALL(m_device, newCommandBufferPool()).WillByDefault([this]() {
            auto pool = std::make_unique<testing::NiceMock<MockCommandBufferPool>>();
            ON_CALL(*pool, get()).WillByDefault(testing::Return(m_commandBuffer));
            return pool;
        });
        ON_CALL(*m_commandBuffer, copyBufferToBuffer(testing::_, testing::_, testing::_)).WillByDefault([](const std::shared_ptr<gfx::Buffer>& src, const std::shared_ptr<gfx::Buffer>& dst, size_t size) {
            dst->setContent(src->content<std::byte>(), size);
        });
    }

    testing::NiceMock<MockDevice> m_device;
    std::shared_ptr<testing::NiceMock<MockCommandBuffer>> m_commandBuffer;
};

std::filesystem::path dummyTexturePath()
{
    return std::filesystem::path(GE_TEST_RESOURCE_DIR) / "dummy_texture.png";
}

std::filesystem::path dummyMeshPath()
{
    return std::filesystem::path(GE_TEST_RESOURCE_DIR) / "triangle.gltf";
}

std::filesystem::path transformedDummyMeshPath()
{
    return std::filesystem::path(GE_TEST_RESOURCE_DIR) / "triangle_transformed.gltf";
}

std::filesystem::path noSceneDummyMeshPath()
{
    return std::filesystem::path(GE_TEST_RESOURCE_DIR) / "triangle_no_scene.gltf";
}

std::filesystem::path missingPositionDummyMeshPath()
{
    return std::filesystem::path(GE_TEST_RESOURCE_DIR) / "triangle_missing_position.gltf";
}

std::vector<std::byte> readBinaryFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("failed to open file: " + path.string());

    const std::streamsize size = static_cast<std::streamsize>(std::filesystem::file_size(path));
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    file.read(reinterpret_cast<char*>(bytes.data()), size); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    if (!file)
        throw std::runtime_error("failed to read file: " + path.string());
    return bytes;
}

std::vector<std::pair<std::optional<GE::VAssetPath>, GE::AssetID>> registeredAssets(std::initializer_list<std::pair<std::optional<GE::VAssetPath>, GE::AssetID>> assets)
{
    return std::vector<std::pair<std::optional<GE::VAssetPath>, GE::AssetID>>(assets);
}

const std::pair<std::optional<GE::VAssetPath>, GE::AssetID>& findRegisteredAsset(const GE::Scene::Descriptor& descriptor, GE::AssetID assetId)
{
    const auto it = std::ranges::find_if(descriptor.registredAssets, [assetId](const auto& asset) {
        return asset.second == assetId;
    });
    if (it == descriptor.registredAssets.end())
        throw std::runtime_error("asset id not found in descriptor");
    return *it;
}

TEST_F(AssetManagerMockDeviceTest, reportsTextureLoadedStateAndMetadata)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(texturePath);

    EXPECT_NE(assetId, GE::BUILT_IN_CUBE_ID);
    EXPECT_TRUE(assetManager.isValidAssetId(assetId));
    EXPECT_TRUE(assetManager.is<gfx::Texture>(assetId));
    EXPECT_FALSE(assetManager.isAssetLoaded(assetId));
    EXPECT_EQ(assetManager.assetLoadCount(assetId), 0u);
    EXPECT_EQ(assetManager.assetName(assetId), "dummy_texture");
    ASSERT_TRUE(assetManager.assetPath(assetId).has_value());
    EXPECT_EQ(*assetManager.assetPath(assetId), texturePath);

    const std::shared_ptr<gfx::Texture> texture = assetManager.loadAsset<gfx::Texture>(assetId).get();
    ASSERT_NE(texture, nullptr);
    EXPECT_TRUE(assetManager.isAssetLoaded(assetId));
    EXPECT_EQ(assetManager.assetLoadCount(assetId), 1u);
    EXPECT_EQ(assetManager.getAsset<gfx::Texture>(assetId), texture);

    assetManager.unloadAsset(assetId);
    EXPECT_FALSE(assetManager.isAssetLoaded(assetId));
    EXPECT_EQ(assetManager.assetLoadCount(assetId), 0u);
}

TEST_F(AssetManagerMockDeviceTest, registeringTheSamePathReturnsTheSameAssetId)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID firstId = assetManager.registerAsset<gfx::Texture>(texturePath);
    const GE::AssetID secondId = assetManager.registerAsset<gfx::Texture>(texturePath);

    EXPECT_EQ(secondId, firstId);
}

TEST_F(AssetManagerMockDeviceTest, explicitAssetIdsArePreservedAndCanBeReusedForTheSamePath)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID explicitId = 42;
    const GE::AssetID registeredId = assetManager.registerAsset<gfx::Texture>(texturePath, explicitId);
    const GE::AssetID registeredAgainId = assetManager.registerAsset<gfx::Texture>(texturePath, explicitId);
    const GE::AssetID autoAssignedMeshId = assetManager.registerAsset<GE::Mesh>(dummyMeshPath());

    EXPECT_EQ(registeredId, explicitId);
    EXPECT_EQ(registeredAgainId, explicitId);
    EXPECT_NE(autoAssignedMeshId, explicitId);
    EXPECT_TRUE(assetManager.isValidAssetId(explicitId));
    ASSERT_TRUE(assetManager.assetPath(explicitId).has_value());
    EXPECT_EQ(*assetManager.assetPath(explicitId), texturePath);
}

TEST_F(AssetManagerMockDeviceTest, reportsBuiltInCubeLoadedStateThroughView)
{
    GE::AssetManager assetManager(&m_device);
    GE::AssetManagerView assetManagerView(&assetManager, registeredAssets({
        { std::nullopt, GE::BUILT_IN_CUBE_ID }
    }));

    EXPECT_FALSE(assetManagerView.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));

    assetManagerView.load();
    ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);

    EXPECT_TRUE(assetManagerView.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);
    EXPECT_EQ(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID)->subMeshes.size(), 1u);

    assetManagerView.unload();
    EXPECT_FALSE(assetManagerView.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 0u);
}

TEST_F(AssetManagerMockDeviceTest, failedLoadCanBeRetriedAfterUnloadResetsTheHandle)
{
    const testing::Matcher<const std::shared_ptr<gfx::CommandBuffer>&> commandBufferMatcher = testing::_;

    testing::InSequence sequence;
    EXPECT_CALL(m_device, submitCommandBuffers(commandBufferMatcher))
        .WillOnce(testing::Throw(std::runtime_error("submit failed")));
    EXPECT_CALL(m_device, submitCommandBuffers(commandBufferMatcher))
        .WillOnce([](const std::shared_ptr<gfx::CommandBuffer>&) {});

    GE::AssetManager assetManager(&m_device);

    EXPECT_ANY_THROW(assetManager.loadAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID).get());
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);

    assetManager.unloadAsset(GE::BUILT_IN_CUBE_ID);
    EXPECT_FALSE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 0u);

    EXPECT_NO_THROW(assetManager.loadAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID).get());
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);

    assetManager.unloadAsset(GE::BUILT_IN_CUBE_ID);
}

TEST_F(AssetManagerMockDeviceTest, meshAssetLoadsFromFileAndPreservesPathMetadata)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID meshAssetId = assetManager.registerAsset<GE::Mesh>(meshPath);

    const std::shared_ptr<GE::Mesh> mesh = assetManager.loadAsset<GE::Mesh>(meshAssetId).get();
    ASSERT_NE(mesh, nullptr);
    ASSERT_EQ(mesh->subMeshes.size(), 1u);
    EXPECT_EQ(mesh->subMeshes.front().name, "triangle0");
    ASSERT_NE(mesh->subMeshes.front().vertexBuffer, nullptr);
    ASSERT_NE(mesh->subMeshes.front().indexBuffer, nullptr);
    ASSERT_EQ(mesh->subMeshes.front().vertexBuffer->size(), sizeof(GE::Vertex) * 3);
    ASSERT_EQ(mesh->subMeshes.front().indexBuffer->size(), sizeof(uint32_t) * 3);
    const GE::Vertex* vertices = mesh->subMeshes.front().vertexBuffer->content<GE::Vertex>();
    const uint32_t* indices = mesh->subMeshes.front().indexBuffer->content<uint32_t>();
    ASSERT_NE(vertices, nullptr);
    ASSERT_NE(indices, nullptr);
    EXPECT_FLOAT_EQ(vertices[0].uv.x, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].uv.y, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].normal.x, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].normal.y, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].normal.z, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].tangent.x, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].tangent.y, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].tangent.z, 0.0f);
    EXPECT_FLOAT_EQ(vertices[0].tangent.w, 0.0f);
    EXPECT_EQ(indices[0], 0u);
    EXPECT_EQ(indices[1], 1u);
    EXPECT_EQ(indices[2], 2u);
    EXPECT_TRUE(assetManager.isAssetLoaded(meshAssetId));
    EXPECT_EQ(assetManager.assetName(meshAssetId), "triangle");
    ASSERT_TRUE(assetManager.assetPath(meshAssetId).has_value());
    EXPECT_EQ(*assetManager.assetPath(meshAssetId), meshPath);

    assetManager.unloadAsset(meshAssetId);
    EXPECT_FALSE(assetManager.isAssetLoaded(meshAssetId));
}

TEST_F(AssetManagerMockDeviceTest, transformedMeshAssetComposesParentAndChildTranslations)
{
    const std::filesystem::path meshPath = transformedDummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetLoader<GE::Mesh> loader(&m_device, meshPath);
    const std::shared_ptr<GE::Mesh> mesh = loader.load(*m_commandBuffer);

    ASSERT_NE(mesh, nullptr);
    ASSERT_EQ(mesh->subMeshes.size(), 1u);
    const glm::mat4& transform = mesh->subMeshes.front().transform;
    EXPECT_FLOAT_EQ(transform[3][0], 5.0f);
    EXPECT_FLOAT_EQ(transform[3][1], 7.0f);
    EXPECT_FLOAT_EQ(transform[3][2], 9.0f);
}

TEST_F(AssetManagerMockDeviceTest, meshLoaderThrowsForFilesWithoutScenes)
{
    const std::filesystem::path meshPath = noSceneDummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetLoader<GE::Mesh> loader(&m_device, meshPath);
    EXPECT_THROW(loader.load(*m_commandBuffer), std::runtime_error);
}

TEST_F(AssetManagerMockDeviceTest, meshLoaderThrowsWhenPositionAccessorIsMissing)
{
    const std::filesystem::path meshPath = missingPositionDummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetLoader<GE::Mesh> loader(&m_device, meshPath);
    EXPECT_THROW(loader.load(*m_commandBuffer), std::runtime_error);
}

TEST_F(AssetManagerMockDeviceTest, textureLoaderSupportsEncodedImageBytes)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    const std::vector<std::byte> encodedBytes = readBinaryFile(texturePath);
    GE::AssetLoader<gfx::Texture> fileLoader(&m_device, texturePath);
    GE::AssetLoader<gfx::Texture> encodedLoader(&m_device, std::span<const std::byte>(encodedBytes));

    const std::shared_ptr<gfx::Texture> fileTexture = fileLoader.load(*m_commandBuffer);
    const std::shared_ptr<gfx::Texture> encodedTexture = encodedLoader.load(*m_commandBuffer);

    ASSERT_NE(fileTexture, nullptr);
    ASSERT_NE(encodedTexture, nullptr);
    EXPECT_EQ(encodedTexture->width(), fileTexture->width());
    EXPECT_EQ(encodedTexture->height(), fileTexture->height());
    EXPECT_EQ(encodedTexture->pixelFormat(), fileTexture->pixelFormat());
}

TEST_F(AssetManagerMockDeviceTest, textureLoaderSupportsRawPixelBytes)
{
    const std::array<std::byte, 8> rgbaBytes = {
        std::byte{0xff}, std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
        std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}
    };

    GE::AssetLoader<gfx::Texture> loader(&m_device, rgbaBytes.data(), 2, 1);
    const std::shared_ptr<gfx::Texture> texture = loader.load(*m_commandBuffer);

    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->width(), 2u);
    EXPECT_EQ(texture->height(), 1u);
    EXPECT_EQ(texture->pixelFormat(), gfx::PixelFormat::RGBA8Unorm);
}

TEST_F(AssetManagerMockDeviceTest, areAssetsLoadedTracksLoadedSubsets)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath);
    const GE::AssetID meshAssetId = assetManager.registerAsset<GE::Mesh>(meshPath);
    const auto assetIds = std::to_array<GE::AssetID>({ textureAssetId, meshAssetId });

    EXPECT_FALSE(assetManager.areAssetsLoaded(assetIds));

    assetManager.loadAsset<gfx::Texture>(textureAssetId).get();

    EXPECT_TRUE(assetManager.areAssetsLoaded(std::to_array<GE::AssetID>({ textureAssetId })));
    EXPECT_FALSE(assetManager.areAssetsLoaded(assetIds));

    assetManager.loadAsset<GE::Mesh>(meshAssetId).get();

    EXPECT_TRUE(assetManager.areAssetsLoaded(assetIds));

    assetManager.unloadAsset(meshAssetId);
    assetManager.unloadAsset(textureAssetId);
    EXPECT_FALSE(assetManager.areAssetsLoaded(assetIds));
}

TEST_F(AssetManagerMockDeviceTest, sceneLoadAndUnloadAffectsEveryRegisteredAsset)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath);
    GE::Scene scene(&assetManager, GE::Scene::Descriptor{
        .name = "test_scene",
        .activeCamera = INVALID_ENTITY_ID,
        .registredAssets = registeredAssets({
            { std::nullopt, GE::BUILT_IN_CUBE_ID },
            { std::nullopt, textureAssetId }
        }),
        .entities = {}
    });

    const auto assetIds = std::to_array<GE::AssetID>({ GE::BUILT_IN_CUBE_ID, textureAssetId });
    EXPECT_FALSE(scene.isLoaded());
    EXPECT_FALSE(assetManager.areAssetsLoaded(assetIds));

    scene.load();
    ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);
    ASSERT_NE(assetManager.getAsset<gfx::Texture>(textureAssetId), nullptr);

    EXPECT_TRUE(scene.isLoaded());
    EXPECT_TRUE(assetManager.areAssetsLoaded(assetIds));

    scene.unload();

    EXPECT_FALSE(scene.isLoaded());
    EXPECT_FALSE(assetManager.areAssetsLoaded(assetIds));
}

TEST_F(AssetManagerMockDeviceTest, sharedAssetsRemainLoadedUntilAllViewsUnload)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath);
    const auto assets = registeredAssets({
        { std::nullopt, GE::BUILT_IN_CUBE_ID },
        { std::nullopt, textureAssetId }
    });

    GE::AssetManagerView firstView(&assetManager, assets);
    GE::AssetManagerView secondView(&assetManager, assets);

    firstView.load();
    secondView.load();
    ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);
    ASSERT_NE(assetManager.getAsset<gfx::Texture>(textureAssetId), nullptr);

    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 2u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 2u);

    firstView.unload();

    EXPECT_FALSE(firstView.isLoaded());
    EXPECT_TRUE(secondView.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 1u);

    secondView.unload();

    EXPECT_FALSE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 0u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 0u);
}

TEST_F(AssetManagerMockDeviceTest, repeatedViewLoadAndUnloadAreIdempotent)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath);
    GE::AssetManagerView assetManagerView(&assetManager, registeredAssets({
        { std::nullopt, textureAssetId }
    }));

    assetManagerView.load();
    assetManagerView.load();
    ASSERT_NE(assetManager.getAsset<gfx::Texture>(textureAssetId), nullptr);

    EXPECT_TRUE(assetManagerView.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 1u);

    assetManagerView.unload();

    EXPECT_FALSE(assetManagerView.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 0u);
}

TEST_F(AssetManagerMockDeviceTest, addingAssetsToALoadedViewLoadsThemInBackground)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID existingTextureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath, 91);
    GE::AssetManagerView assetManagerView(&assetManager, registeredAssets({
        { std::nullopt, GE::BUILT_IN_CUBE_ID }
    }));

    assetManagerView.load();
    ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);
    ASSERT_TRUE(assetManagerView.isLoaded());

    const GE::AssetID newlyRegisteredAssetId = assetManagerView.registerAsset<GE::Mesh>(meshPath);
    assetManagerView.registerAssetId(existingTextureAssetId);

    EXPECT_FALSE(assetManagerView.isLoaded());

    ASSERT_NE(assetManager.getAsset<GE::Mesh>(newlyRegisteredAssetId), nullptr);
    ASSERT_NE(assetManager.getAsset<gfx::Texture>(existingTextureAssetId), nullptr);
    EXPECT_TRUE(assetManagerView.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(newlyRegisteredAssetId));
    EXPECT_TRUE(assetManager.isAssetLoaded(existingTextureAssetId));

    assetManagerView.unload();
}

TEST_F(AssetManagerMockDeviceTest, moveOperationsPreserveLoadedAssetsUntilTheDestinationUnloads)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath);

    GE::AssetManagerView source(&assetManager, registeredAssets({
        { std::nullopt, textureAssetId }
    }));
    source.load();
    ASSERT_NE(assetManager.getAsset<gfx::Texture>(textureAssetId), nullptr);

    GE::AssetManagerView moved(std::move(source));
    EXPECT_FALSE(source.isLoaded());
    EXPECT_TRUE(moved.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetId));

    GE::AssetManagerView destination(&assetManager);
    destination = std::move(moved);
    EXPECT_FALSE(moved.isLoaded());
    EXPECT_TRUE(destination.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetId));

    destination.unload();
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetId));
}

TEST_F(AssetManagerMockDeviceTest, failedViewLoadCompletesWithAnExceptionAndMarksTheViewLoaded)
{
    const testing::Matcher<const std::shared_ptr<gfx::CommandBuffer>&> commandBufferMatcher = testing::_;

    EXPECT_CALL(m_device, submitCommandBuffers(commandBufferMatcher))
        .WillOnce(testing::Throw(std::runtime_error("submit failed")));

    GE::AssetManager assetManager(&m_device);
    GE::AssetManagerView assetManagerView(&assetManager, registeredAssets({
        { std::nullopt, GE::BUILT_IN_CUBE_ID }
    }));

    EXPECT_NO_THROW(assetManagerView.load());
    EXPECT_THROW(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), std::runtime_error);
    EXPECT_TRUE(assetManagerView.isLoaded());

    assetManagerView.unload();
    EXPECT_FALSE(assetManagerView.isLoaded());
}

TEST_F(AssetManagerMockDeviceTest, sceneDescriptorPreservesOptionalPathsAndAssetIds)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::Scene scene(&assetManager, "scene");
    scene.assetManagerView().registerAssetId(GE::BUILT_IN_CUBE_ID);
    const GE::AssetID textureAssetId = scene.assetManagerView().registerAsset<gfx::Texture>(texturePath);

    const GE::Scene::Descriptor descriptor = scene.makeDescriptor();
    ASSERT_EQ(descriptor.registredAssets.size(), 2u);

    const auto& builtInAsset = findRegisteredAsset(descriptor, GE::BUILT_IN_CUBE_ID);
    EXPECT_FALSE(builtInAsset.first.has_value());

    const auto& textureAsset = findRegisteredAsset(descriptor, textureAssetId);
    ASSERT_TRUE(textureAsset.first.has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetPath<gfx::Texture>>(*textureAsset.first));
    EXPECT_EQ(std::get<GE::AssetPath<gfx::Texture>>(*textureAsset.first).path, texturePath);
}

TEST_F(AssetManagerMockDeviceTest, sceneDescriptorUsesExplicitSerializedAssetIdsForPathBackedAssets)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    GE::Scene scene(&assetManager, GE::Scene::Descriptor{
        .name = "scene",
        .activeCamera = INVALID_ENTITY_ID,
        .registredAssets = registeredAssets({
            { GE::AssetPath<GE::Mesh>(meshPath), 77 }
        }),
        .entities = {
            { 5, { GE::MeshComponent{77} } }
        }
    });

    EXPECT_TRUE(scene.assetManagerView().assets().contains(77));
    EXPECT_TRUE(assetManager.isValidAssetId(77));
    EXPECT_EQ(std::get<GE::MeshComponent>(scene.makeDescriptor().entities.front().second.front()).id, 77u);

    scene.load();
    ASSERT_NE(assetManager.getAsset<GE::Mesh>(77), nullptr);
    EXPECT_TRUE(assetManager.isAssetLoaded(77));

    const GE::Scene::Descriptor descriptor = scene.makeDescriptor();
    const auto& registeredAsset = findRegisteredAsset(descriptor, 77);
    ASSERT_TRUE(registeredAsset.first.has_value());
    EXPECT_EQ(std::get<GE::AssetPath<GE::Mesh>>(*registeredAsset.first).path, meshPath);

    scene.unload();
}

} // namespace

} // namespace GE_tests

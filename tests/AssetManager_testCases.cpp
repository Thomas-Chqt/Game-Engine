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
#include "Game-Engine/Game.hpp"
#include "Game-Engine/MeshAssetLoader.hpp"
#include "Game-Engine/Scene.hpp"
#include "Game-Engine/ScriptLibrary.hpp"
#include "Game-Engine/TextureAssetLoader.hpp"

#include <Graphics/Buffer.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/CommandBufferPool.hpp>
#include <Graphics/Device.hpp>
#include <Graphics/Texture.hpp>

#include <atomic>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <initializer_list>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
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

GE::Scene::Descriptor makeSceneDescriptor(
    std::string name,
    std::initializer_list<std::pair<GE::EntityID, std::vector<GE::ComponentVariant>>> entities = {},
    GE::EntityID activeCameraId = GE::INVALID_ENTITY_ID)
{
    GE::Scene::Descriptor descriptor{
        .name = std::move(name),
        .activeCameraId = activeCameraId
    };

    for (const auto& [entityId, components] : entities)
    {
        descriptor.ecsWorld.registerEntityID(entityId);
        GE::Entity entity{&descriptor.ecsWorld, entityId};
        for (const GE::ComponentVariant& component : components)
        {
            std::visit([&]<typename ComponentT>(const ComponentT& value) {
                entity.emplace<ComponentT>(value);
            }, component);
        }
    }

    return descriptor;
}

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

TEST_F(AssetManagerMockDeviceTest, locationBackedRegistrationUsesLocationIdentity)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID meshAssetId = assetManager.registerAsset<GE::Mesh>(meshPath.stem().string(), meshPath, 0);
    const GE::AssetLocation<gfx::Texture> textureLocation{meshPath, 0};
    const GE::AssetID embeddedTextureId = assetManager.registerAsset<gfx::Texture>(meshPath.stem().string(), meshPath, 0);
    const GE::AssetID embeddedTextureAgainId = assetManager.registerAsset<gfx::Texture>(meshPath.stem().string(), meshPath, 0);

    EXPECT_EQ(embeddedTextureId, embeddedTextureAgainId);
    EXPECT_NE(meshAssetId, embeddedTextureId);
    EXPECT_TRUE(assetManager.isRegistered(GE::VAssetLocation(textureLocation)));
    EXPECT_EQ(assetManager.assetId(GE::VAssetLocation(textureLocation)), embeddedTextureId);
    ASSERT_TRUE(assetManager.assetLocation(embeddedTextureId).has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(embeddedTextureId)));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(embeddedTextureId)).containerPath, textureLocation.containerPath);
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(embeddedTextureId)).index, textureLocation.index);
}

TEST_F(AssetManagerMockDeviceTest, reportsTextureLoadedStateAndMetadata)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);

    EXPECT_NE(assetId, GE::BUILT_IN_CUBE_ID);
    EXPECT_TRUE(assetManager.isValidAssetId(assetId));
    EXPECT_TRUE(assetManager.assetTypeIs<gfx::Texture>(assetId));
    EXPECT_FALSE(assetManager.isAssetLoaded(assetId));
    EXPECT_EQ(assetManager.assetLoadCount(assetId), 0u);
    EXPECT_EQ(assetManager.assetName(assetId), "dummy_texture");
    ASSERT_TRUE(assetManager.assetLocation(assetId).has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(assetId)));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(assetId)).containerPath, texturePath);
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(assetId)).index, 0u);

    const std::shared_ptr<gfx::Texture> texture = assetManager.loadAsset<gfx::Texture>(assetId).get();
    ASSERT_NE(texture, nullptr);
    EXPECT_TRUE(assetManager.isAssetLoaded(assetId));
    EXPECT_EQ(assetManager.assetLoadCount(assetId), 1u);
    EXPECT_EQ(assetManager.getAsset<gfx::Texture>(assetId), texture);

    assetManager.unloadAsset(assetId);
    EXPECT_FALSE(assetManager.isAssetLoaded(assetId));
    EXPECT_EQ(assetManager.assetLoadCount(assetId), 0u);
}

TEST_F(AssetManagerMockDeviceTest, reportsAssetIdsAndTypesForDebugInspection)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    const std::filesystem::path meshPath = dummyMeshPath();

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const GE::AssetID meshAssetId = assetManager.registerAsset<GE::Mesh>(meshPath.stem().string(), meshPath);

    EXPECT_THAT(assetManager.assetIds(), testing::ElementsAre(GE::BUILT_IN_CUBE_ID, textureAssetId, meshAssetId));
    EXPECT_TRUE(assetManager.assetTypeIs<GE::Mesh>(GE::BUILT_IN_CUBE_ID));
    EXPECT_TRUE(assetManager.assetTypeIs<gfx::Texture>(textureAssetId));
    EXPECT_TRUE(assetManager.assetTypeIs<GE::Mesh>(meshAssetId));
    EXPECT_TRUE(assetManager.isRegistered(GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{texturePath, 0})));
    EXPECT_TRUE(assetManager.isRegistered(GE::VAssetLocation(GE::AssetLocation<GE::Mesh>{meshPath, 0})));
    EXPECT_EQ(assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{texturePath, 0})), textureAssetId);
    EXPECT_EQ(assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<GE::Mesh>{meshPath, 0})), meshAssetId);
}

TEST_F(AssetManagerMockDeviceTest, registeringTheSamePathReturnsTheSameAssetId)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID firstId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const GE::AssetID secondId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);

    EXPECT_EQ(secondId, firstId);
}

TEST_F(AssetManagerMockDeviceTest, explicitAssetIdsArePreservedAndCanBeReusedForTheSamePath)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID explicitId = 42;
    const std::filesystem::path meshPath = dummyMeshPath();
    const GE::AssetID registeredId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath, 0, explicitId);
    const GE::AssetID registeredAgainId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath, 0, explicitId);
    const GE::AssetID autoAssignedMeshId = assetManager.registerAsset<GE::Mesh>(meshPath.stem().string(), meshPath);

    EXPECT_EQ(registeredId, explicitId);
    EXPECT_EQ(registeredAgainId, explicitId);
    EXPECT_NE(autoAssignedMeshId, explicitId);
    EXPECT_TRUE(assetManager.isValidAssetId(explicitId));
    ASSERT_TRUE(assetManager.assetLocation(explicitId).has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(explicitId)));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(explicitId)).containerPath, texturePath);
}

TEST_F(AssetManagerMockDeviceTest, loaderBackedAssetRegistrationPreservesExplicitNameAndHasNoPath)
{
    const std::array<std::byte, 8> rgbaBytes = {
        std::byte{0xff}, std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
        std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}
    };

    GE::AssetManager assetManager(&m_device);
    constexpr GE::AssetID explicitId = 73;
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(
        "generated_texture",
        GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, rgbaBytes.data(), 2, 1),
        explicitId
    );
    const GE::AssetID registeredAgainId = assetManager.registerAsset<gfx::Texture>(
        "generated_texture",
        GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, rgbaBytes.data(), 2, 1),
        explicitId
    );

    EXPECT_EQ(assetId, explicitId);
    EXPECT_EQ(registeredAgainId, explicitId);
    EXPECT_TRUE(assetManager.isValidAssetId(assetId));
    EXPECT_TRUE(assetManager.assetTypeIs<gfx::Texture>(assetId));
    EXPECT_EQ(assetManager.assetName(assetId), "generated_texture");
    EXPECT_FALSE(assetManager.assetLocation(assetId).has_value());

    const std::shared_ptr<gfx::Texture> texture = assetManager.loadAsset<gfx::Texture>(assetId).get();
    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(texture->width(), 2u);
    EXPECT_EQ(texture->height(), 1u);

    assetManager.unloadAsset(assetId);
}

TEST_F(AssetManagerMockDeviceTest, loaderBackedAssetRegistrationAutoAssignedIdsRemainUniqueAcrossThreads)
{
    const std::array<std::byte, 8> rgbaBytes = {
        std::byte{0xff}, std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
        std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}
    };

    GE::AssetManager assetManager(&m_device);

    constexpr size_t workerCount = 8;
    std::atomic<bool> start = false;
    std::array<std::future<GE::AssetID>, workerCount> futures;

    for (auto& future : futures) {
        future = std::async(std::launch::async, [&]() {
            while (start.load(std::memory_order_acquire) == false)
                std::this_thread::yield();
            return assetManager.registerAsset<gfx::Texture>(
                "generated_texture",
                GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, rgbaBytes.data(), 2, 1)
            );
        });
    }

    start.store(true, std::memory_order_release);

    std::set<GE::AssetID> registeredAssetIds;
    for (auto& future : futures) {
        const GE::AssetID assetId = future.get();
        registeredAssetIds.insert(assetId);
        EXPECT_TRUE(assetManager.isValidAssetId(assetId));
        EXPECT_TRUE(assetManager.assetTypeIs<gfx::Texture>(assetId));
        EXPECT_EQ(assetManager.assetName(assetId), "generated_texture");
        EXPECT_FALSE(assetManager.assetLocation(assetId).has_value());
    }

    EXPECT_EQ(registeredAssetIds.size(), workerCount);
    EXPECT_FALSE(registeredAssetIds.contains(GE::BUILT_IN_CUBE_ID));
}

TEST_F(AssetManagerMockDeviceTest, concurrentRegisterAssetReturnsTheSameAssetId)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);

    constexpr size_t workerCount = 8;
    std::atomic<bool> start = false;
    std::array<std::future<GE::AssetID>, workerCount> futures;

    for (auto& future : futures) {
        future = std::async(std::launch::async, [&]() {
            while (start.load(std::memory_order_acquire) == false)
                std::this_thread::yield();
            return assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
        });
    }

    start.store(true, std::memory_order_release);

    const GE::AssetID registeredId = futures.front().get();
    for (size_t i = 1; i < futures.size(); ++i)
        EXPECT_EQ(futures[i].get(), registeredId);
}

TEST_F(AssetManagerMockDeviceTest, concurrentPublicAccessDoesNotRequireExternalSynchronization)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const auto assetIds = std::to_array<GE::AssetID>({ assetId });

    constexpr size_t workerCount = 8;
    std::atomic<bool> start = false;
    std::array<std::future<void>, workerCount> loadFutures;

    for (size_t i = 0; auto& future : loadFutures) {
        future = std::async(std::launch::async, [&, i]() {
            while (start.load(std::memory_order_acquire) == false)
                std::this_thread::yield();
            switch (i % 5) {
            case 0:
                ASSERT_NE(assetManager.loadAsset<gfx::Texture>(assetId).get(), nullptr);
                break;
            case 1:
                assetManager.loadAsset(assetId).get();
                break;
            case 2:
                assetManager.loadAssets(assetIds).get();
                break;
            case 3:
                assetManager.loadAssetsDetached(assetIds);
                ASSERT_NE(assetManager.getAsset<gfx::Texture>(assetId), nullptr);
                break;
            case 4:
                assetManager.loadAssetsDetached(assetIds);
                ASSERT_NE(assetManager.getAsset<gfx::Texture>(assetId), nullptr);
                break;
            default:
                std::unreachable();
            }
        });
        ++i;
    }

    auto observeAssetState = [&]() {
        while (start.load(std::memory_order_acquire) == false)
            std::this_thread::yield();

        for (size_t i = 0; i < 128; ++i) {
            EXPECT_THAT(assetManager.assetIds(), testing::Contains(assetId));
            EXPECT_TRUE(assetManager.isRegistered(GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{texturePath, 0})));
            EXPECT_EQ(assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{texturePath, 0})), assetId);
            EXPECT_TRUE(assetManager.isValidAssetId(assetId));
            EXPECT_TRUE(assetManager.assetTypeIs<gfx::Texture>(assetId));
            EXPECT_EQ(assetManager.assetName(assetId), "dummy_texture");
            ASSERT_TRUE(assetManager.assetLocation(assetId).has_value());
            ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(assetId)));
            EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(assetId)).containerPath, texturePath);
            EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(assetId)).index, 0u);
            (void)assetManager.isAssetLoaded(assetId);
            (void)assetManager.areAssetsLoaded(assetIds);
            (void)assetManager.assetLoadCount(assetId);
        }
    };

    std::future<void> observer = std::async(std::launch::async, observeAssetState);

    start.store(true, std::memory_order_release);

    for (auto& future : loadFutures)
        future.get();

    observer.get();

    EXPECT_TRUE(assetManager.isAssetLoaded(assetId));
    EXPECT_EQ(assetManager.assetLoadCount(assetId), workerCount);

    start.store(false, std::memory_order_release);
    std::array<std::future<void>, workerCount> unloadFutures;
    for (size_t i = 0; auto& future : unloadFutures) {
        future = std::async(std::launch::async, [&, i]() {
            while (start.load(std::memory_order_acquire) == false)
                std::this_thread::yield();
            if (i % 2 == 0)
                assetManager.unloadAsset(assetId);
            else
                assetManager.unloadAssets(assetIds);
        });
        ++i;
    }

    std::future<void> unloadObserver = std::async(std::launch::async, observeAssetState);

    start.store(true, std::memory_order_release);

    for (auto& future : unloadFutures)
        future.get();
    unloadObserver.get();

    EXPECT_FALSE(assetManager.isAssetLoaded(assetId));
    EXPECT_EQ(assetManager.assetLoadCount(assetId), 0u);
}

TEST_F(AssetManagerMockDeviceTest, loadAssetsLoadsEveryAssetInTheRange)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const auto assetIds = std::to_array<GE::AssetID>({ GE::BUILT_IN_CUBE_ID, textureAssetId });

    assetManager.loadAssets(assetIds).get();

    EXPECT_TRUE(assetManager.areAssetsLoaded(assetIds));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 1u);

    assetManager.unloadAssets(assetIds);
}

TEST_F(AssetManagerMockDeviceTest, reloadingAnAlreadyLoadedAssetDoesNotCreateANewCommandBufferPool)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);

    EXPECT_CALL(m_device, newCommandBufferPool()).Times(1);

    ASSERT_NE(assetManager.loadAsset<gfx::Texture>(textureAssetId).get(), nullptr);
    ASSERT_NE(assetManager.loadAsset<gfx::Texture>(textureAssetId).get(), nullptr);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 2u);

    assetManager.unloadAsset(textureAssetId);
    assetManager.unloadAsset(textureAssetId);
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
    const GE::AssetID meshAssetId = assetManager.registerAsset<GE::Mesh>(meshPath.stem().string(), meshPath);

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
    ASSERT_TRUE(assetManager.assetLocation(meshAssetId).has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<GE::Mesh>>(*assetManager.assetLocation(meshAssetId)));
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(*assetManager.assetLocation(meshAssetId)).containerPath, meshPath);
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(*assetManager.assetLocation(meshAssetId)).index, 0u);

    assetManager.unloadAsset(meshAssetId);
    EXPECT_FALSE(assetManager.isAssetLoaded(meshAssetId));
}

TEST_F(AssetManagerMockDeviceTest, transformedMeshAssetComposesParentAndChildTranslations)
{
    const std::filesystem::path meshPath = transformedDummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    GE::AssetLoader<GE::Mesh> loader(&m_device, &assetManager, GE::AssetLocation<GE::Mesh>{meshPath, 0});
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

    GE::AssetManager assetManager(&m_device);
    GE::AssetLoader<GE::Mesh> loader(&m_device, &assetManager, GE::AssetLocation<GE::Mesh>{meshPath, 0});
    EXPECT_THROW(loader.load(*m_commandBuffer), std::runtime_error);
}

TEST_F(AssetManagerMockDeviceTest, meshLoaderThrowsWhenPositionAccessorIsMissing)
{
    const std::filesystem::path meshPath = missingPositionDummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    GE::AssetLoader<GE::Mesh> loader(&m_device, &assetManager, GE::AssetLocation<GE::Mesh>{meshPath, 0});
    EXPECT_THROW(loader.load(*m_commandBuffer), std::runtime_error);
}

TEST_F(AssetManagerMockDeviceTest, textureLoaderSupportsEncodedImageBytes)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    const std::vector<std::byte> encodedBytes = readBinaryFile(texturePath);
    GE::AssetManager assetManager(&m_device);
    GE::AssetLoader<gfx::Texture> fileLoader(&m_device, &assetManager, GE::AssetLocation<gfx::Texture>{texturePath, 0});
    GE::AssetLoader<gfx::Texture> encodedLoader(&m_device, nullptr, std::span<const std::byte>(encodedBytes));

    const std::shared_ptr<gfx::Texture> fileTexture = fileLoader.load(*m_commandBuffer);
    const std::shared_ptr<gfx::Texture> encodedTexture = encodedLoader.load(*m_commandBuffer);

    ASSERT_NE(fileTexture, nullptr);
    ASSERT_NE(encodedTexture, nullptr);
    EXPECT_EQ(encodedTexture->width(), fileTexture->width());
    EXPECT_EQ(encodedTexture->height(), fileTexture->height());
    EXPECT_EQ(encodedTexture->pixelFormat(), fileTexture->pixelFormat());
}

TEST_F(AssetManagerMockDeviceTest, textureLoaderSupportsGltfContainerTextureIndex)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetLocation<gfx::Texture> textureLocation{meshPath, 0};
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(meshPath.stem().string(), meshPath, 0);

    const std::shared_ptr<gfx::Texture> texture = assetManager.loadAsset<gfx::Texture>(textureAssetId).get();
    const std::shared_ptr<gfx::Texture> fileTexture = GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, GE::AssetLocation<gfx::Texture>{texturePath, 0}).load(*m_commandBuffer);

    ASSERT_NE(texture, nullptr);
    ASSERT_NE(fileTexture, nullptr);
    EXPECT_EQ(texture->width(), fileTexture->width());
    EXPECT_EQ(texture->height(), fileTexture->height());
    ASSERT_TRUE(assetManager.assetLocation(textureAssetId).has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(textureAssetId)));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(textureAssetId)).containerPath, textureLocation.containerPath);
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(textureAssetId)).index, textureLocation.index);
}

TEST_F(AssetManagerMockDeviceTest, textureLoaderSupportsRawPixelBytes)
{
    const std::array<std::byte, 8> rgbaBytes = {
        std::byte{0xff}, std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
        std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}
    };

    GE::AssetLoader<gfx::Texture> loader(&m_device, nullptr, rgbaBytes.data(), 2, 1);
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
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const GE::AssetID meshAssetId = assetManager.registerAsset<GE::Mesh>(meshPath.stem().string(), meshPath);
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

TEST_F(AssetManagerMockDeviceTest, sceneConstructionLoadsMeshAssetsReferencedByTheEcsWorld)
{
    GE::AssetManager assetManager(&m_device);
    GE::Scene scene(&assetManager, makeSceneDescriptor("test_scene", {
        { 1, { GE::MeshComponent{GE::BUILT_IN_CUBE_ID} } }
    }));

    ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);
}

TEST_F(AssetManagerMockDeviceTest, sceneDestructionUnloadsMeshAssetsReferencedByTheEcsWorld)
{
    GE::AssetManager assetManager(&m_device);
    {
        GE::Scene scene(&assetManager, makeSceneDescriptor("test_scene", {
            { 1, { GE::MeshComponent{GE::BUILT_IN_CUBE_ID} } }
        }));

        ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);
        EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    }

    EXPECT_FALSE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 0u);
}

TEST_F(AssetManagerMockDeviceTest, sceneConstructionCountsDuplicateMeshReferences)
{
    GE::AssetManager assetManager(&m_device);
    GE::Scene scene(&assetManager, makeSceneDescriptor("test_scene", {
        { 1, { GE::MeshComponent{GE::BUILT_IN_CUBE_ID} } },
        { 2, { GE::MeshComponent{GE::BUILT_IN_CUBE_ID} } }
    }));

    ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 2u);
}

TEST_F(AssetManagerMockDeviceTest, sceneLoadsPreRegisteredMeshAssetsByExplicitId)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID meshAssetId = assetManager.registerAsset<GE::Mesh>(meshPath.stem().string(), meshPath, 0, 77);
    GE::Scene scene(&assetManager, makeSceneDescriptor("scene", {
        { 5, { GE::MeshComponent{meshAssetId} } }
    }));

    EXPECT_EQ(meshAssetId, 77u);
    EXPECT_TRUE(assetManager.isValidAssetId(meshAssetId));
    ASSERT_NE(assetManager.getAsset<GE::Mesh>(meshAssetId), nullptr);
    EXPECT_TRUE(assetManager.isAssetLoaded(meshAssetId));
    ASSERT_TRUE(assetManager.assetLocation(meshAssetId).has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<GE::Mesh>>(*assetManager.assetLocation(meshAssetId)));
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(*assetManager.assetLocation(meshAssetId)).containerPath, meshPath);
    EXPECT_EQ(std::get<GE::AssetLocation<GE::Mesh>>(*assetManager.assetLocation(meshAssetId)).index, 0u);
}

TEST_F(AssetManagerMockDeviceTest, gameLoadSceneConstructsARuntimeSceneWithoutMakingItActive)
{
    GE::AssetManager assetManager(&m_device);
    GE::Game game(GE::Game::Descriptor{
        .assetManager = &assetManager,
        .inputContext = {},
        .scenes = {
            makeSceneDescriptor("first"),
            makeSceneDescriptor("second", {
                { 2, { GE::MeshComponent{GE::BUILT_IN_CUBE_ID} } }
            })
        },
        .startSceneName = "first",
        .scriptLibrary = nullptr
    });

    EXPECT_EQ(game.activeScene().name(), "first");
    EXPECT_FALSE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));

    game.loadScene("second");
    ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);

    EXPECT_EQ(game.activeScene().name(), "first");
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);
}

TEST_F(AssetManagerMockDeviceTest, gameSetActiveSceneRequiresTheSceneToBeLoaded)
{
    GE::AssetManager assetManager(&m_device);
    GE::Game game(GE::Game::Descriptor{
        .assetManager = &assetManager,
        .inputContext = {},
        .scenes = {
            makeSceneDescriptor("first"),
            makeSceneDescriptor("second")
        },
        .startSceneName = "first",
        .scriptLibrary = nullptr
    });

    EXPECT_DEATH(game.setActiveScene("second"), "");
}

TEST_F(AssetManagerMockDeviceTest, gameSwitchingActiveScenesKeepsLoadedAssetsResidentUntilExplicitUnload)
{
    GE::AssetManager assetManager(&m_device);
    GE::Game game(GE::Game::Descriptor{
        .assetManager = &assetManager,
        .inputContext = {},
        .scenes = {
            makeSceneDescriptor("first", {
                { 1, { GE::MeshComponent{GE::BUILT_IN_CUBE_ID} } }
            }),
            makeSceneDescriptor("second")
        },
        .startSceneName = "first",
        .scriptLibrary = nullptr
    });

    ASSERT_NE(assetManager.getAsset<GE::Mesh>(GE::BUILT_IN_CUBE_ID), nullptr);
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));

    game.loadScene("second");
    game.setActiveScene("second");

    EXPECT_EQ(game.activeScene().name(), "second");
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);

    game.unloadScene("first");

    EXPECT_FALSE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 0u);
}

TEST_F(AssetManagerMockDeviceTest, gameUnloadingTheActiveSceneAsserts)
{
    GE::AssetManager assetManager(&m_device);
    GE::Game game(GE::Game::Descriptor{
        .assetManager = &assetManager,
        .inputContext = {},
        .scenes = {
            makeSceneDescriptor("first")
        },
        .startSceneName = "first",
        .scriptLibrary = nullptr
    });

    EXPECT_DEATH(game.unloadScene("first"), "");
}

TEST_F(AssetManagerMockDeviceTest, gameSceneActivationRunsScriptSetupAndTeardownWithoutUnloadingTheScene)
{
    GE::AssetManager assetManager(&m_device);
    GE::ScriptLibrary scriptLibrary(GE_TEST_SCRIPT_LIB);
    GE::Game game(GE::Game::Descriptor{
        .assetManager = &assetManager,
        .inputContext = {},
        .scenes = {
            makeSceneDescriptor("first", {
                { 1, {
                    GE::TransformComponent{},
                    GE::ScriptComponent{ .name = "TestScript" }
                }}
            }),
            makeSceneDescriptor("second")
        },
        .startSceneName = "first",
        .scriptLibrary = &scriptLibrary
    });

    ASSERT_EQ(game.activeScene().ecsWorld().get<GE::ScriptComponent>(1).instance != nullptr, true);

    game.loadScene("second");
    game.setActiveScene("second");

    EXPECT_EQ(game.activeScene().name(), "second");
}

} // namespace

} // namespace GE_tests

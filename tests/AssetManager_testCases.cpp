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
#include "Game-Engine/Scene.hpp"

#include <Graphics/Buffer.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/CommandBufferPool.hpp>
#include <Graphics/Device.hpp>
#include <Graphics/Texture.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <thread>
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

GE::VAssetPath builtInCubeAssetPath()
{
    return GE::AssetPath<GE::Mesh>(GE::BUILT_IN_CUBE_PATH);
}

bool waitUntil(const std::function<bool()>& predicate, std::chrono::milliseconds timeout = std::chrono::seconds(1))
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (predicate())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return predicate();
}

TEST_F(AssetManagerMockDeviceTest, reportsTextureLoadedState)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::VAssetPath assetPath = GE::AssetPath<gfx::Texture>(texturePath);

    assetManager.registerAsset(assetPath);

    EXPECT_FALSE(assetManager.isAssetLoaded(assetPath));

    assetManager.loadAsset<gfx::Texture>(assetPath).get();
    EXPECT_TRUE(assetManager.isAssetLoaded(assetPath));

    assetManager.unloadAsset(assetPath);
    EXPECT_FALSE(assetManager.isAssetLoaded(assetPath));
}

TEST_F(AssetManagerMockDeviceTest, reportsBuiltInCubeLoadedState)
{
    GE::AssetManager assetManager(&m_device);
    GE::AssetManagerView assetManagerView(&assetManager, {
        { 0, builtInCubeAssetPath() }
    });

    EXPECT_FALSE(assetManagerView.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(builtInCubeAssetPath()));

    assetManagerView.load().get();
    EXPECT_TRUE(assetManagerView.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_NE(assetManagerView.getAsset<GE::Mesh>(0), nullptr);

    assetManagerView.unload();
    EXPECT_FALSE(assetManagerView.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
}

TEST_F(AssetManagerMockDeviceTest, failedLoadCanBeRetriedWithoutLeakingRefCount)
{
    const testing::Matcher<const std::shared_ptr<gfx::CommandBuffer>&> commandBufferMatcher = testing::_;

    testing::InSequence sequence;
    EXPECT_CALL(m_device, submitCommandBuffers(commandBufferMatcher))
        .WillOnce(testing::Throw(std::runtime_error("submit failed")));
    EXPECT_CALL(m_device, submitCommandBuffers(commandBufferMatcher))
        .WillRepeatedly([](const std::shared_ptr<gfx::CommandBuffer>&) {});

    GE::AssetManager assetManager(&m_device);
    const GE::VAssetPath assetPath = builtInCubeAssetPath();

    EXPECT_THROW(assetManager.loadAsset<GE::Mesh>(assetPath).get(), std::runtime_error);
    EXPECT_FALSE(assetManager.isAssetLoaded(assetPath));

    EXPECT_NO_THROW(assetManager.loadAsset<GE::Mesh>(assetPath).get());
    EXPECT_TRUE(assetManager.isAssetLoaded(assetPath));

    assetManager.unloadAsset(assetPath);
    EXPECT_FALSE(assetManager.isAssetLoaded(assetPath));
}

TEST_F(AssetManagerMockDeviceTest, loadAndUnloadAffectsAllRegisteredAssets)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::VAssetPath textureAssetPath = GE::AssetPath<gfx::Texture>(texturePath);
    assetManager.registerAsset(textureAssetPath);

    constexpr GE::AssetID builtInCubeAssetId = 0;
    constexpr GE::AssetID textureAssetId = 42;
    GE::Scene scene(&assetManager, GE::Scene::Descriptor{
        .name = "test_scene",
        .activeCamera = INVALID_ENTITY_ID,
        .registredAssets = {
            { builtInCubeAssetId, builtInCubeAssetPath() },
            { textureAssetId, textureAssetPath }
        },
        .entities = {}
    });

    EXPECT_FALSE(scene.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetPath));

    scene.load().get();

    EXPECT_TRUE(scene.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetPath));
    EXPECT_NE(scene.assetManagerView().getAsset<GE::Mesh>(builtInCubeAssetId), nullptr);
    EXPECT_NE(scene.assetManagerView().getAsset<gfx::Texture>(textureAssetId), nullptr);

    scene.unload();

    EXPECT_FALSE(scene.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetPath));
}

TEST_F(AssetManagerMockDeviceTest, sharedAssetsRemainLoadedUntilAllViewsUnload)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::VAssetPath textureAssetPath = GE::AssetPath<gfx::Texture>(texturePath);
    assetManager.registerAsset(textureAssetPath);

    constexpr GE::AssetID builtInCubeAssetId = 0;
    constexpr GE::AssetID textureAssetId = 42;
    const std::map<GE::AssetID, GE::VAssetPath> registredAssets = {
        { builtInCubeAssetId, builtInCubeAssetPath() },
        { textureAssetId, textureAssetPath }
    };
    GE::AssetManagerView firstView(&assetManager, registredAssets);
    GE::AssetManagerView secondView(&assetManager, registredAssets);

    firstView.load().get();
    secondView.load().get();

    EXPECT_TRUE(firstView.isLoaded());
    EXPECT_TRUE(secondView.isLoaded());
    EXPECT_TRUE(firstView.areAllAssetsLoaded());
    EXPECT_TRUE(secondView.areAllAssetsLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetPath));

    firstView.unload();

    EXPECT_FALSE(firstView.isLoaded());
    EXPECT_TRUE(secondView.isLoaded());
    EXPECT_TRUE(firstView.areAllAssetsLoaded());
    EXPECT_TRUE(secondView.areAllAssetsLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetPath));

    secondView.unload();

    EXPECT_FALSE(secondView.isLoaded());
    EXPECT_FALSE(secondView.areAllAssetsLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetPath));
}

TEST_F(AssetManagerMockDeviceTest, reportsSubsetAssetLoadedStateThroughAssetIds)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::VAssetPath textureAssetPath = GE::AssetPath<gfx::Texture>(texturePath);
    assetManager.registerAsset(textureAssetPath);

    constexpr GE::AssetID builtInCubeAssetId = 0;
    constexpr GE::AssetID textureAssetId = 42;
    GE::AssetManagerView assetManagerView(&assetManager, {
        { builtInCubeAssetId, builtInCubeAssetPath() },
        { textureAssetId, textureAssetPath }
    });

    EXPECT_FALSE(assetManagerView.areAssetsLoaded(std::to_array<GE::AssetID>({ builtInCubeAssetId, textureAssetId })));
    EXPECT_FALSE(assetManagerView.areAssetsLoaded(std::to_array<GE::AssetID>({ textureAssetId })));

    assetManagerView.load().get();

    EXPECT_TRUE(assetManagerView.areAssetsLoaded(std::to_array<GE::AssetID>({ builtInCubeAssetId, textureAssetId })));
    EXPECT_TRUE(assetManagerView.areAssetsLoaded(std::to_array<GE::AssetID>({ textureAssetId })));
}

TEST_F(AssetManagerMockDeviceTest, repeatedViewLoadAndUnloadAreIdempotent)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::VAssetPath textureAssetPath = GE::AssetPath<gfx::Texture>(texturePath);
    assetManager.registerAsset(textureAssetPath);

    GE::AssetManagerView assetManagerView(&assetManager, {
        { 1, textureAssetPath }
    });

    assetManagerView.load().get();
    assetManagerView.load().get();
    EXPECT_TRUE(assetManagerView.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetPath));

    assetManagerView.unload();
    EXPECT_FALSE(assetManagerView.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetPath));

    assetManagerView.unload();
    EXPECT_FALSE(assetManagerView.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetPath));
}

TEST_F(AssetManagerMockDeviceTest, registeringAssetOnLoadedViewLoadsNewAssetWithoutInvalidatingView)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::VAssetPath textureAssetPath = GE::AssetPath<gfx::Texture>(texturePath);
    assetManager.registerAsset(textureAssetPath);

    GE::AssetManagerView assetManagerView(&assetManager, {
        { 0, builtInCubeAssetPath() }
    });

    assetManagerView.load().get();
    EXPECT_TRUE(assetManagerView.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(builtInCubeAssetPath()));

    const GE::AssetID textureAssetId = assetManagerView.registerAsset<gfx::Texture>(texturePath);
    EXPECT_TRUE(assetManagerView.isLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_TRUE(waitUntil([&] { return assetManager.isAssetLoaded(textureAssetPath); }));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetPath));
    EXPECT_NE(assetManagerView.getAsset<gfx::Texture>(textureAssetId), nullptr);

    assetManagerView.unload();
    EXPECT_FALSE(assetManagerView.isLoaded());
    EXPECT_FALSE(assetManager.isAssetLoaded(builtInCubeAssetPath()));
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetPath));
}

TEST_F(AssetManagerMockDeviceTest, moveConstructorTransfersLoadedStateWithoutDroppingAssets)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::VAssetPath textureAssetPath = GE::AssetPath<gfx::Texture>(texturePath);
    assetManager.registerAsset(textureAssetPath);

    GE::AssetManagerView original(&assetManager, {
        { 1, textureAssetPath }
    });
    original.load().get();

    GE::AssetManagerView moved(std::move(original));

    EXPECT_FALSE(original.isLoaded());
    EXPECT_TRUE(moved.isLoaded());
    EXPECT_TRUE(moved.areAllAssetsLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetPath));

    moved.unload();
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetPath));
}

TEST_F(AssetManagerMockDeviceTest, moveAssignmentTransfersLoadedStateWithoutDroppingAssets)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    GE::VAssetPath textureAssetPath = GE::AssetPath<gfx::Texture>(texturePath);
    assetManager.registerAsset(textureAssetPath);

    GE::AssetManagerView source(&assetManager, {
        { 1, textureAssetPath }
    });
    source.load().get();

    GE::AssetManagerView destination(&assetManager, {});
    destination = std::move(source);

    EXPECT_FALSE(source.isLoaded());
    EXPECT_TRUE(destination.isLoaded());
    EXPECT_TRUE(destination.areAllAssetsLoaded());
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetPath));

    destination.unload();
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetPath));
}

} // namespace

} // namespace GE_tests

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
#include "../src/TextureTable.hpp"

#include <Graphics/Buffer.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/CommandBufferPool.hpp>
#include <Graphics/Device.hpp>
#include <Graphics/ParameterBlock.hpp>
#include <Graphics/ParameterBlockLayout.hpp>
#include <Graphics/ParameterBlockPool.hpp>
#include <Graphics/Sampler.hpp>
#include <Graphics/Texture.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace GE_tests
{

namespace
{

constexpr uint32_t TEXTURE_TABLE_CAPACITY = 4096;

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

class TestSampler final : public gfx::Sampler
{
};

class TestParameterBlockLayout final : public gfx::ParameterBlockLayout
{
public:
    explicit TestParameterBlockLayout(Descriptor desc)
        : m_desc(std::move(desc))
    {
    }

    const std::vector<gfx::ParameterBlockBinding>& bindings() const override { return m_desc.bindings; }

private:
    Descriptor m_desc;
};

class MockParameterBlock : public gfx::ParameterBlock
{
public:
    MOCK_METHOD(std::shared_ptr<gfx::ParameterBlockLayout>, layout, (), (const, override));
    MOCK_METHOD(void, setBinding, (uint32_t, (const std::shared_ptr<gfx::Buffer>&)), (override));
    MOCK_METHOD(void, setBinding, (uint32_t, (const std::shared_ptr<gfx::Texture>&)), (override));
    MOCK_METHOD(void, setBinding, (uint32_t, uint32_t, (const std::shared_ptr<gfx::Texture>&)), (override));
    MOCK_METHOD(void, setBinding, (uint32_t, uint32_t, std::span<const std::shared_ptr<gfx::Texture>>), (override));
    MOCK_METHOD(void, setBinding, (uint32_t, (const std::shared_ptr<gfx::Sampler>&)), (override));
    MOCK_METHOD(void, clearBinding, (uint32_t, uint32_t), (override));
    MOCK_METHOD(void, clearBinding, (uint32_t, uint32_t, uint32_t), (override));
};

class TestParameterBlockPool final : public gfx::ParameterBlockPool
{
public:
    explicit TestParameterBlockPool(std::shared_ptr<gfx::ParameterBlock> parameterBlock)
        : m_parameterBlock(std::move(parameterBlock))
    {
    }

    std::shared_ptr<gfx::ParameterBlock> get(const std::shared_ptr<gfx::ParameterBlockLayout>&) override { return m_parameterBlock; }
    void reset() override {}

private:
    std::shared_ptr<gfx::ParameterBlock> m_parameterBlock;
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
        : m_textureParameterBlock(std::make_shared<testing::NiceMock<MockParameterBlock>>())
        , m_commandBuffer(std::make_shared<testing::NiceMock<MockCommandBuffer>>())
    {
        ON_CALL(m_device, backend()).WillByDefault(testing::Return(gfx::Backend::metal));
        ON_CALL(m_device, newParameterBlockLayout(testing::_)).WillByDefault([](const gfx::ParameterBlockLayout::Descriptor& desc) {
            return std::make_unique<TestParameterBlockLayout>(desc);
        });
        ON_CALL(m_device, newParameterBlockPool(testing::_)).WillByDefault([this](const gfx::ParameterBlockPool::Descriptor& desc) {
            m_parameterBlockPoolDescriptors.push_back(desc);
            return std::make_unique<TestParameterBlockPool>(m_textureParameterBlock);
        });
        ON_CALL(m_device, newSampler(testing::_)).WillByDefault([](const gfx::Sampler::Descriptor&) {
            return std::make_unique<TestSampler>();
        });
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
    std::shared_ptr<testing::NiceMock<MockParameterBlock>> m_textureParameterBlock;
    std::shared_ptr<gfx::ParameterBlockLayout> m_textureParameterBlockLayout;
    std::vector<gfx::ParameterBlockPool::Descriptor> m_parameterBlockPoolDescriptors;
    std::shared_ptr<testing::NiceMock<MockCommandBuffer>> m_commandBuffer;

    std::shared_ptr<GE::TextureTable> makeTextureTable()
    {
        m_textureParameterBlockLayout = m_device.newParameterBlockLayout({
            .bindings = {
                { .type = gfx::BindingType::sampler,        .usages = gfx::BindingUsage::fragmentRead },
                { .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::fragmentRead, .count = TEXTURE_TABLE_CAPACITY },
            }
        });
        ON_CALL(*m_textureParameterBlock, layout()).WillByDefault(testing::Return(m_textureParameterBlockLayout));

        std::unique_ptr<gfx::ParameterBlockPool> textureTableBlockPool = m_device.newParameterBlockPool({
            .maxBindingCount = {
                { gfx::BindingType::sampler, 1 },
                { gfx::BindingType::sampledTexture, TEXTURE_TABLE_CAPACITY },
            },
            .updateAfterBind = true
        });
        std::shared_ptr<gfx::ParameterBlock> textureTableBlock = textureTableBlockPool->get(m_textureParameterBlockLayout);
        textureTableBlock->setBinding(0, std::shared_ptr<gfx::Sampler>(m_device.newSampler(gfx::Sampler::Descriptor{})));

        return std::make_shared<GE::TextureTable>(textureTableBlock.get(), 1);
    }

    std::shared_ptr<GE::TextureTable> attachTextureTable(GE::AssetManager& assetManager)
    {
        std::shared_ptr<GE::TextureTable> textureTable = makeTextureTable();
        assetManager.attachTextureTable(textureTable);
        return textureTable;
    }
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

    EXPECT_NE(meshAssetId, embeddedTextureId);
    EXPECT_TRUE(assetManager.isRegistered(GE::VAssetLocation(textureLocation)));
    EXPECT_EQ(assetManager.assetId(GE::VAssetLocation(textureLocation)), embeddedTextureId);
    ASSERT_TRUE(assetManager.assetLocation(embeddedTextureId).has_value());
    ASSERT_TRUE(std::holds_alternative<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(embeddedTextureId)));
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(embeddedTextureId)).containerPath, textureLocation.containerPath);
    EXPECT_EQ(std::get<GE::AssetLocation<gfx::Texture>>(*assetManager.assetLocation(embeddedTextureId)).index, textureLocation.index);

    const GE::AssetID embeddedTextureAgainId = assetManager.registerAsset<gfx::Texture>(meshPath.stem().string(), meshPath, 0);
    EXPECT_EQ(embeddedTextureAgainId, embeddedTextureId);
    EXPECT_EQ(assetManager.assetId(GE::VAssetLocation(textureLocation)), embeddedTextureId);
}

TEST_F(AssetManagerMockDeviceTest, importGltfRegistersMeshesTexturesAndMeshDependencies)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    assetManager.importGltf(meshPath);

    const GE::AssetLocation<GE::Mesh> meshLocation{meshPath, 0};
    const GE::AssetLocation<gfx::Texture> textureLocation{meshPath, 0};

    ASSERT_TRUE(assetManager.isRegistered(GE::VAssetLocation(meshLocation)));
    ASSERT_TRUE(assetManager.isRegistered(GE::VAssetLocation(textureLocation)));

    const GE::AssetID meshAssetId = assetManager.assetId(GE::VAssetLocation(meshLocation));
    const GE::AssetID textureAssetId = assetManager.assetId(GE::VAssetLocation(textureLocation));

    EXPECT_THAT(assetManager.assetDependencies(meshAssetId) | std::ranges::to<std::vector>(), testing::ElementsAre(textureAssetId));
    EXPECT_THAT(assetManager.assetDependencies(textureAssetId) | std::ranges::to<std::vector>(), testing::IsEmpty());
}

TEST_F(AssetManagerMockDeviceTest, importGltfReportsOneContainerPathForAllAssetsInTheFile)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    assetManager.importGltf(meshPath);

    EXPECT_THAT(assetManager.assetContainerPaths(), testing::ElementsAre(meshPath));
}

TEST_F(AssetManagerMockDeviceTest, importGltfMeshLoadAlsoLoadsRegisteredTextureDependencies)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    assetManager.importGltf(meshPath);

    const GE::AssetID meshAssetId = assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<GE::Mesh>{meshPath, 0}));
    const GE::AssetID textureAssetId = assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{meshPath, 0}));

    ASSERT_NE(assetManager.loadAsset<GE::Mesh>(meshAssetId).get(), nullptr);
    EXPECT_TRUE(assetManager.isAssetLoaded(meshAssetId));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(meshAssetId), 1u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 1u);
}

TEST_F(AssetManagerMockDeviceTest, loadingImportedMeshTwiceKeepsTextureDependenciesReferencedAfterOneUnload)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    assetManager.importGltf(meshPath);

    const GE::AssetID meshAssetId = assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<GE::Mesh>{meshPath, 0}));
    const GE::AssetID textureAssetId = assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{meshPath, 0}));

    ASSERT_NE(assetManager.loadAsset<GE::Mesh>(meshAssetId).get(), nullptr);
    ASSERT_NE(assetManager.loadAsset<GE::Mesh>(meshAssetId).get(), nullptr);

    EXPECT_TRUE(assetManager.isAssetLoaded(meshAssetId));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(meshAssetId), 2u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 2u);

    assetManager.unloadAsset(meshAssetId);

    EXPECT_TRUE(assetManager.isAssetLoaded(meshAssetId));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(meshAssetId), 1u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 1u);
}

TEST_F(AssetManagerMockDeviceTest, unloadingImportedMeshAlsoUnloadsItsTextureDependencies)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    assetManager.importGltf(meshPath);

    const GE::AssetID meshAssetId = assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<GE::Mesh>{meshPath, 0}));
    const GE::AssetID textureAssetId = assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{meshPath, 0}));

    ASSERT_NE(assetManager.loadAsset<GE::Mesh>(meshAssetId).get(), nullptr);
    ASSERT_TRUE(assetManager.isAssetLoaded(meshAssetId));
    ASSERT_TRUE(assetManager.isAssetLoaded(textureAssetId));
    ASSERT_EQ(assetManager.assetLoadCount(textureAssetId), 1u);

    assetManager.unloadAsset(meshAssetId);

    EXPECT_FALSE(assetManager.isAssetLoaded(meshAssetId));
    EXPECT_FALSE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(meshAssetId), 0u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 0u);
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

TEST_F(AssetManagerMockDeviceTest, createsTextureParameterBlockTable)
{
    EXPECT_CALL(*m_textureParameterBlock, setBinding(0u, testing::An<const std::shared_ptr<gfx::Sampler>&>()));

    const std::shared_ptr<GE::TextureTable> textureTable = makeTextureTable();

    ASSERT_NE(textureTable, nullptr);
    const std::shared_ptr<gfx::ParameterBlockLayout> layout = m_textureParameterBlockLayout;
    ASSERT_NE(layout, nullptr);
    ASSERT_EQ(layout->bindings().size(), 2u);
    EXPECT_EQ(layout->bindings().at(0).type, gfx::BindingType::sampler);
    EXPECT_EQ(layout->bindings().at(0).usages, gfx::BindingUsage::fragmentRead);
    EXPECT_EQ(layout->bindings().at(0).count, 1u);
    EXPECT_EQ(layout->bindings().at(1).type, gfx::BindingType::sampledTexture);
    EXPECT_EQ(layout->bindings().at(1).usages, gfx::BindingUsage::fragmentRead);
    EXPECT_EQ(layout->bindings().at(1).count, TEXTURE_TABLE_CAPACITY);

    ASSERT_EQ(m_parameterBlockPoolDescriptors.size(), 1u);
    EXPECT_TRUE(m_parameterBlockPoolDescriptors.front().updateAfterBind);
    EXPECT_THAT(
        m_parameterBlockPoolDescriptors.front().maxBindingCount,
        testing::ElementsAre(
            std::pair<const gfx::BindingType, uint32_t>{gfx::BindingType::sampledTexture, TEXTURE_TABLE_CAPACITY},
            std::pair<const gfx::BindingType, uint32_t>{gfx::BindingType::sampler, 1u}
        )
    );
}

TEST_F(AssetManagerMockDeviceTest, loadingTextureAssignsTextureTableIndex)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const std::shared_ptr<GE::TextureTable> textureTable = attachTextureTable(assetManager);

    uint32_t boundIndex = std::numeric_limits<uint32_t>::max();
    std::shared_ptr<gfx::Texture> boundTexture;
    EXPECT_CALL(*m_textureParameterBlock, setBinding(1u, testing::_, testing::An<const std::shared_ptr<gfx::Texture>&>()))
        .WillOnce([&](uint32_t, uint32_t textureIndex, const std::shared_ptr<gfx::Texture>& texture) {
            boundIndex = textureIndex;
            boundTexture = texture;
        });

    const std::shared_ptr<gfx::Texture> texture = assetManager.loadAsset<gfx::Texture>(assetId).get();

    ASSERT_NE(texture, nullptr);
    EXPECT_EQ(boundTexture, texture);
    EXPECT_EQ(textureTable->textureIndex(assetId), boundIndex);

    assetManager.unloadAsset(assetId);
}

TEST_F(AssetManagerMockDeviceTest, unloadingTextureClearsTextureTableIndex)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const std::shared_ptr<GE::TextureTable> textureTable = attachTextureTable(assetManager);

    uint32_t boundIndex = std::numeric_limits<uint32_t>::max();
    EXPECT_CALL(*m_textureParameterBlock, setBinding(1u, testing::_, testing::An<const std::shared_ptr<gfx::Texture>&>()))
        .WillOnce([&](uint32_t, uint32_t textureIndex, const std::shared_ptr<gfx::Texture>&) {
            boundIndex = textureIndex;
        });

    ASSERT_NE(assetManager.loadAsset<gfx::Texture>(assetId).get(), nullptr);
    ASSERT_EQ(textureTable->textureIndex(assetId), boundIndex);

    EXPECT_CALL(*m_textureParameterBlock, clearBinding(1u, boundIndex));
    assetManager.unloadAsset(assetId);

    #ifndef NDEBUG
    EXPECT_DEATH({
        [[maybe_unused]] const uint32_t textureIndex = textureTable->textureIndex(assetId);
    }, "");
    #endif
}

TEST_F(AssetManagerMockDeviceTest, failedTextureLoadCanUnloadWithoutClearingTextureTableIndex)
{
    const std::array<std::byte, 4> invalidBytes = {
        std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}
    };

    GE::AssetManager assetManager(&m_device);
    const std::shared_ptr<GE::TextureTable> textureTable = attachTextureTable(assetManager);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(
        std::nullopt,
        "invalid_texture",
        std::nullopt,
        std::array<GE::AssetID, 0>{},
        GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, std::span<const std::byte>(invalidBytes))
    );

    EXPECT_ANY_THROW(assetManager.loadAsset<gfx::Texture>(assetId).get());
    EXPECT_CALL(*m_textureParameterBlock, clearBinding(testing::_, testing::_)).Times(0);
    assetManager.unloadAsset(assetId);
}

TEST_F(AssetManagerMockDeviceTest, attachingTextureTableFillsAlreadyLoadedTextures)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);

    const std::shared_ptr<gfx::Texture> texture = assetManager.loadAsset<gfx::Texture>(assetId).get();
    ASSERT_NE(texture, nullptr);

    const std::shared_ptr<GE::TextureTable> textureTable = makeTextureTable();
    uint32_t boundIndex = std::numeric_limits<uint32_t>::max();
    std::shared_ptr<gfx::Texture> boundTexture;
    EXPECT_CALL(*m_textureParameterBlock, setBinding(1u, testing::_, testing::An<const std::shared_ptr<gfx::Texture>&>()))
        .WillOnce([&](uint32_t, uint32_t textureIndex, const std::shared_ptr<gfx::Texture>& bound) {
            boundIndex = textureIndex;
            boundTexture = bound;
        });

    assetManager.attachTextureTable(textureTable);

    EXPECT_EQ(boundTexture, texture);
    EXPECT_EQ(textureTable->textureIndex(assetId), boundIndex);

    assetManager.unloadAsset(assetId);
}

TEST_F(AssetManagerMockDeviceTest, expiredTextureTableIsNotWrittenAfterTextureLoad)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);

    {
        const std::shared_ptr<GE::TextureTable> textureTable = attachTextureTable(assetManager);
        ASSERT_NE(textureTable, nullptr);
    }

    EXPECT_CALL(*m_textureParameterBlock, setBinding(1u, testing::_, testing::An<const std::shared_ptr<gfx::Texture>&>()))
        .Times(0);
    EXPECT_CALL(*m_textureParameterBlock, clearBinding(testing::_, testing::_)).Times(0);

    ASSERT_NE(assetManager.loadAsset<gfx::Texture>(assetId).get(), nullptr);
    assetManager.unloadAsset(assetId);
}

TEST_F(AssetManagerMockDeviceTest, reloadingTextureCanReuseFreedTextureTableIndex)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const std::shared_ptr<GE::TextureTable> textureTable = attachTextureTable(assetManager);

    std::vector<uint32_t> boundIndices;
    EXPECT_CALL(*m_textureParameterBlock, setBinding(1u, testing::_, testing::An<const std::shared_ptr<gfx::Texture>&>()))
        .Times(2)
        .WillRepeatedly([&](uint32_t, uint32_t textureIndex, const std::shared_ptr<gfx::Texture>&) {
            boundIndices.push_back(textureIndex);
        });

    ASSERT_NE(assetManager.loadAsset<gfx::Texture>(assetId).get(), nullptr);
    ASSERT_EQ(boundIndices.size(), 1u);
    const uint32_t firstIndex = boundIndices.at(0);

    EXPECT_CALL(*m_textureParameterBlock, clearBinding(1u, firstIndex)).RetiresOnSaturation();
    assetManager.unloadAsset(assetId);

    ASSERT_NE(assetManager.loadAsset<gfx::Texture>(assetId).get(), nullptr);
    ASSERT_EQ(boundIndices.size(), 2u);
    EXPECT_EQ(boundIndices.at(1), firstIndex);
}

TEST_F(AssetManagerMockDeviceTest, loadedTexturesUseDifferentTextureTableIndices)
{
    const std::array<std::byte, 8> rgbaBytes = {
        std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255},
        std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255}
    };

    GE::AssetManager assetManager(&m_device);
    const std::shared_ptr<GE::TextureTable> textureTable = attachTextureTable(assetManager);
    const GE::AssetID firstId = assetManager.registerAsset<gfx::Texture>(
        std::nullopt,
        "first_texture",
        std::nullopt,
        std::array<GE::AssetID, 0>{},
        GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, rgbaBytes.data(), 2, 1)
    );
    const GE::AssetID secondId = assetManager.registerAsset<gfx::Texture>(
        std::nullopt,
        "second_texture",
        std::nullopt,
        std::array<GE::AssetID, 0>{},
        GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, rgbaBytes.data(), 2, 1)
    );

    EXPECT_CALL(*m_textureParameterBlock, setBinding(1u, testing::_, testing::An<const std::shared_ptr<gfx::Texture>&>()))
        .Times(2);

    ASSERT_NE(assetManager.loadAsset<gfx::Texture>(firstId).get(), nullptr);
    ASSERT_NE(assetManager.loadAsset<gfx::Texture>(secondId).get(), nullptr);

    EXPECT_NE(textureTable->textureIndex(firstId), textureTable->textureIndex(secondId));

    assetManager.unloadAsset(secondId);
    assetManager.unloadAsset(firstId);
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

TEST_F(AssetManagerMockDeviceTest, importGltfTwiceIsNoOpAndKeepsAssetIdsAndDependenciesStable)
{
    const std::filesystem::path meshPath = dummyMeshPath();
    ASSERT_TRUE(std::filesystem::is_regular_file(meshPath));

    GE::AssetManager assetManager(&m_device);
    const GE::VAssetLocation meshLocation = GE::AssetLocation<GE::Mesh>{meshPath, 0};
    const GE::VAssetLocation textureLocation = GE::AssetLocation<gfx::Texture>{meshPath, 0};

    assetManager.importGltf(meshPath);

    const GE::AssetID firstMeshAssetId = assetManager.assetId(meshLocation);
    const GE::AssetID firstTextureAssetId = assetManager.assetId(textureLocation);
    const std::vector<GE::AssetID> firstDependencies = assetManager.assetDependencies(firstMeshAssetId) | std::ranges::to<std::vector>();
    const std::vector<GE::AssetID> firstAssetIds = assetManager.assetIds() | std::ranges::to<std::vector>();

    assetManager.importGltf(meshPath);

    EXPECT_EQ(assetManager.assetId(meshLocation), firstMeshAssetId);
    EXPECT_EQ(assetManager.assetId(textureLocation), firstTextureAssetId);
    EXPECT_THAT(assetManager.assetDependencies(firstMeshAssetId) | std::ranges::to<std::vector>(), testing::ElementsAreArray(firstDependencies));
    EXPECT_THAT(assetManager.assetIds(), testing::ElementsAreArray(firstAssetIds));
}

TEST_F(AssetManagerMockDeviceTest, registeringTheSamePathTwiceKeepsTheFirstAssetId)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID firstId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const GE::AssetID secondId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);

    EXPECT_EQ(secondId, firstId);
    EXPECT_EQ(assetManager.assetId(GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{texturePath, 0})), firstId);
}

TEST_F(AssetManagerMockDeviceTest, explicitAssetIdsArePreserved)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID explicitId = 42;
    const std::filesystem::path meshPath = dummyMeshPath();
    const GE::AssetID registeredId = assetManager.registerAsset(
        explicitId,
        texturePath.stem().string(),
        GE::VAssetLocation(GE::AssetLocation<gfx::Texture>{texturePath, 0}),
        std::array<GE::AssetID, 0>{}
    );
    const GE::AssetID autoAssignedMeshId = assetManager.registerAsset<GE::Mesh>(meshPath.stem().string(), meshPath);

    EXPECT_EQ(registeredId, explicitId);
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
        explicitId,
        "generated_texture",
        std::optional<GE::AssetLocation<gfx::Texture>>{},
        std::array<GE::AssetID, 0>{},
        GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, rgbaBytes.data(), 2, 1)
    );

    EXPECT_EQ(assetId, explicitId);
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

TEST_F(AssetManagerMockDeviceTest, loaderBackedAssetRegistrationAutoAssignedIdsRemainUnique)
{
    const std::array<std::byte, 8> rgbaBytes = {
        std::byte{0xff}, std::byte{0x00}, std::byte{0x00}, std::byte{0xff},
        std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}
    };

    GE::AssetManager assetManager(&m_device);

    std::set<GE::AssetID> registeredAssetIds;
    for (size_t i = 0; i < 8; ++i) {
        const GE::AssetID assetId = assetManager.registerAsset<gfx::Texture>(
            std::nullopt,
            "generated_texture",
            std::optional<GE::AssetLocation<gfx::Texture>>{},
            std::array<GE::AssetID, 0>{},
            GE::AssetLoader<gfx::Texture>(&m_device, &assetManager, rgbaBytes.data(), 2, 1)
        );
        registeredAssetIds.insert(assetId);
        EXPECT_TRUE(assetManager.isValidAssetId(assetId));
        EXPECT_TRUE(assetManager.assetTypeIs<gfx::Texture>(assetId));
        EXPECT_EQ(assetManager.assetName(assetId), "generated_texture");
        EXPECT_FALSE(assetManager.assetLocation(assetId).has_value());
    }

    EXPECT_EQ(registeredAssetIds.size(), 8u);
    EXPECT_FALSE(registeredAssetIds.contains(GE::BUILT_IN_CUBE_ID));
}

TEST_F(AssetManagerMockDeviceTest, loadAssetsLoadsEveryAssetInTheRange)
{
    const std::filesystem::path texturePath = dummyTexturePath();
    ASSERT_TRUE(std::filesystem::is_regular_file(texturePath));

    GE::AssetManager assetManager(&m_device);
    const GE::AssetID textureAssetId = assetManager.registerAsset<gfx::Texture>(texturePath.stem().string(), texturePath);
    const auto assetIds = std::to_array<GE::AssetID>({ GE::BUILT_IN_CUBE_ID, textureAssetId });

    assetManager.loadAssets(assetIds).get();

    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_TRUE(assetManager.isAssetLoaded(textureAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);
    EXPECT_EQ(assetManager.assetLoadCount(textureAssetId), 1u);

    assetManager.unloadAssets(assetIds);
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
        EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);
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
    const GE::AssetID meshAssetId = assetManager.registerAsset(
        GE::AssetID{77},
        meshPath.stem().string(),
        GE::VAssetLocation(GE::AssetLocation<GE::Mesh>{meshPath, 0}),
        std::array<GE::AssetID, 0>{}
    );
    GE::Scene scene(&assetManager, makeSceneDescriptor("scene", {
        { 5, { GE::MeshComponent{meshAssetId} } }
    }));

    EXPECT_EQ(meshAssetId, 77u);
    EXPECT_TRUE(assetManager.isValidAssetId(meshAssetId));
    ASSERT_NE(assetManager.getAsset<GE::Mesh>(meshAssetId), nullptr);
    EXPECT_TRUE(assetManager.isAssetLoaded(meshAssetId));
    EXPECT_EQ(assetManager.assetLoadCount(meshAssetId), 1u);
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
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);

    game.loadScene("second");
    game.setActiveScene("second");

    EXPECT_EQ(game.activeScene().name(), "second");
    EXPECT_TRUE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 1u);

    game.unloadScene("first");

    EXPECT_FALSE(assetManager.isAssetLoaded(GE::BUILT_IN_CUBE_ID));
    EXPECT_EQ(assetManager.assetLoadCount(GE::BUILT_IN_CUBE_ID), 0u);
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

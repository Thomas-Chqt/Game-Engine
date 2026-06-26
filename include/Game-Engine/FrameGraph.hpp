/*
 * ---------------------------------------------------
 * FrameGraph.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef FRAMEGRAPH_HPP
#define FRAMEGRAPH_HPP

#include "Game-Engine/AssetManager.hpp"

#include <Graphics/Buffer.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/Enums.hpp>
#include <Graphics/GraphicsPipeline.hpp>
#include <Graphics/ParameterBlock.hpp>
#include <Graphics/ParameterBlockLayout.hpp>
#include <Graphics/ParameterBlockPool.hpp>
#include <Graphics/Texture.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace GE
{

struct FramePass;

struct FrameGraph
{
    struct Texture
    {
        gfx::Texture::Descriptor descriptor;
    };

    struct Buffer
    {
        gfx::Buffer::Descriptor descriptor;
        size_t offset = 0; // offset into buffersContent where the data starts
    };

    using TextureRef = size_t;
    using BufferRef = size_t;

    struct Attachment
    {
        TextureRef texture;
        gfx::LoadAction loadAction;
        union {
            std::array<float, 4> clearColor;
            float clearDepth;
        };
    };

    std::vector<std::byte> buffersContent;

    std::vector<Texture> textures;
    std::vector<Buffer> buffers;
    TextureRef backBuffer = std::numeric_limits<TextureRef>::max();
    std::vector<FramePass> passes;
};

struct FramePass
{
    struct ExecuteContext
    {
        AssetManager& assetManager;
        gfx::CommandBuffer& commandBuffer;
        gfx::ParameterBlockPool& parameterBlockPool;
        std::shared_ptr<gfx::ParameterBlock> textureTableBlock;
        std::shared_ptr<gfx::ParameterBlockLayout> frameDataBlockLayout;

        std::function<std::shared_ptr<gfx::Texture>(FrameGraph::TextureRef)> texture;
        std::function<std::shared_ptr<gfx::Buffer>(FrameGraph::BufferRef)> buffer;
        std::shared_ptr<gfx::ParameterBlockLayout> texturedMaterialPBlockLayout;
        std::shared_ptr<gfx::GraphicsPipeline> texturedPipeline;
    };

    std::vector<FrameGraph::Attachment> colorAttachments;
    std::optional<FrameGraph::Attachment> depthAttachment;

    std::vector<FrameGraph::TextureRef> sampledTextures;

    std::function<void(ExecuteContext&)> execute;
};

class TextureTable;

class FrameGraphBuilder
{
public:
    FrameGraphBuilder() = delete;
    FrameGraphBuilder(const FrameGraphBuilder&) = delete;
    FrameGraphBuilder(FrameGraphBuilder&&) = default;

    FrameGraphBuilder(TextureTable*, AssetManager*);

    FrameGraph::TextureRef newTexture(const gfx::Texture::Descriptor&);
    FrameGraph::TextureRef newTexture(std::pair<uint32_t, uint32_t> size, gfx::PixelFormat);
    FrameGraph::TextureRef newTexture(std::string name, const gfx::Texture::Descriptor&);
    FrameGraph::TextureRef newTexture(std::string name, std::pair<uint32_t, uint32_t> size, gfx::PixelFormat);
    void aliasTexture(std::string name, FrameGraph::TextureRef);
    FrameGraph::TextureRef texture(std::string_view name) const;
    const std::map<std::string, FrameGraph::TextureRef, std::less<>>& textureNames() const;

    template<typename T>
    FrameGraph::BufferRef newConstantBuffer() requires std::is_trivially_copyable_v<T>;

    template<typename T>
    FrameGraph::BufferRef newStructuredBuffer(size_t count) requires std::is_trivially_copyable_v<T>;

    void setBackBuffer(FrameGraph::TextureRef);

    template<typename T>
    T& constantBufferContent(FrameGraph::BufferRef) requires std::is_trivially_copyable_v<T>;

    template<typename T>
    std::span<T> structuredBufferContent(FrameGraph::BufferRef) requires std::is_trivially_copyable_v<T>;

    void addPass(FramePass);

    AssetManager& assetManager() const;
    uint32_t textureIndex(AssetID) const;

    FrameGraph build() &&;

    ~FrameGraphBuilder() = default;

private:
    TextureTable* m_textureTable;
    AssetManager* m_assetManager;
    FrameGraph m_frameGraph;
    std::map<std::string, FrameGraph::TextureRef, std::less<>> m_textureNames;
    size_t m_nextOffset = 0;

public:
    FrameGraphBuilder& operator=(const FrameGraphBuilder&) = delete;
    FrameGraphBuilder& operator=(FrameGraphBuilder&&) = default;

};

template<typename T>
FrameGraph::BufferRef FrameGraphBuilder::newConstantBuffer() requires std::is_trivially_copyable_v<T>
{
    const size_t remainder = m_nextOffset % alignof(T);
    const size_t offset = remainder == 0 ? m_nextOffset : m_nextOffset + alignof(T) - remainder;
    m_frameGraph.buffersContent.resize(offset + sizeof(T));

    FrameGraph::BufferRef buffer = m_frameGraph.buffers.size();
    m_frameGraph.buffers.emplace_back(gfx::Buffer::Descriptor{
        .size = sizeof(T),
        .usages = gfx::BufferUsage::constantBuffer,
        .storageMode = gfx::ResourceStorageMode::hostVisible
    }, offset);
    m_nextOffset = offset + sizeof(T);

    return buffer;
}

template<typename T>
FrameGraph::BufferRef FrameGraphBuilder::newStructuredBuffer(size_t count) requires std::is_trivially_copyable_v<T>
{
    assert(count > 0);
    const size_t size = count * sizeof(T);
    const size_t remainder = m_nextOffset % alignof(T);
    const size_t offset = remainder == 0 ? m_nextOffset : m_nextOffset + alignof(T) - remainder;
    m_frameGraph.buffersContent.resize(offset + size);

    FrameGraph::BufferRef buffer = m_frameGraph.buffers.size();
    m_frameGraph.buffers.emplace_back(gfx::Buffer::Descriptor{
        .size = size,
        .usages = gfx::BufferUsage::structuredBuffer,
        .storageMode = gfx::ResourceStorageMode::hostVisible
    }, offset);
    m_nextOffset = offset + size;

    return buffer;
}

template<typename T>
T& FrameGraphBuilder::constantBufferContent(FrameGraph::BufferRef buffRef) requires std::is_trivially_copyable_v<T>
{
    assert(buffRef < m_frameGraph.buffers.size());
    const FrameGraph::Buffer& buffer = m_frameGraph.buffers[buffRef];
    assert(buffer.descriptor.size >= sizeof(T));
    assert(buffer.offset + sizeof(T) <= m_frameGraph.buffersContent.size());

    std::byte* ptr = m_frameGraph.buffersContent.data() + buffer.offset;
    assert(reinterpret_cast<std::uintptr_t>(ptr) % alignof(T) == 0); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    return *reinterpret_cast<T*>(ptr); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
}

template<typename T>
std::span<T> FrameGraphBuilder::structuredBufferContent(FrameGraph::BufferRef buffRef) requires std::is_trivially_copyable_v<T>
{
    assert(buffRef < m_frameGraph.buffers.size());
    const FrameGraph::Buffer& buffer = m_frameGraph.buffers[buffRef];
    assert(buffer.descriptor.size % sizeof(T) == 0);
    assert(buffer.offset + buffer.descriptor.size <= m_frameGraph.buffersContent.size());

    std::byte* ptr = m_frameGraph.buffersContent.data() + buffer.offset;
    assert(reinterpret_cast<std::uintptr_t>(ptr) % alignof(T) == 0); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::span<T>(
        reinterpret_cast<T*>(ptr), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
        buffer.descriptor.size / sizeof(T)
    );
}

} // namespace GE

#endif

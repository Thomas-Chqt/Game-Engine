/*
 * ---------------------------------------------------
 * FrameGraph.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef FRAMEGRAPH_HPP
#define FRAMEGRAPH_HPP

#include "Game-Engine/Export.hpp"

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/ParameterBlockPool.hpp>
#include <Graphics/Enums.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Buffer.hpp>

#include <concepts>
#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include <array>

namespace GE
{

struct FramePass;

template<class T>
concept FramePassBuilder = requires(const T& b) {
    { b.build() } -> std::convertible_to<FramePass>;
};

struct TextureDescriptor
{
    std::string name;
    std::pair<uint32_t, uint32_t> size;
    gfx::PixelFormat pixelFormat;
};

struct ConstantBufferDescriptor
{
    std::string name;
    uint32_t size;
};

struct StructuredBufferDescriptor
{
    std::string name;
};

struct AttachmentDescriptor
{
    std::string texture;
    gfx::LoadAction loadAction;
    union {
        std::array<float, 4> clearColor;
        float clearDepth;
    };
};

struct FramePassSetupContext
{
    std::map<std::string, std::shared_ptr<gfx::Texture>>& textureMap;
    std::map<std::string, std::shared_ptr<gfx::Buffer>>& constantBuffers;
    std::function<void(const std::string& name, const void* data, uint32_t size)> setStructuredBufferContent;
};

struct FramePassExecuteContext
{
    gfx::CommandBuffer& commandBuffer;
    gfx::ParameterBlockPool& parameterBlockPool;
    std::map<std::string, std::shared_ptr<gfx::Texture>>& textureMap;
    std::map<std::string, std::shared_ptr<gfx::Buffer>>& bufferMap;

    std::shared_ptr<gfx::ParameterBlockLayout> frameDataBlockLayout;
    std::shared_ptr<gfx::ParameterBlockLayout> materialBlockLayout;
    std::shared_ptr<gfx::GraphicsPipeline> gfxPipeline; // only one for now
};

struct FramePass
{
    std::vector<AttachmentDescriptor> colorAttachments;
    std::optional<AttachmentDescriptor> depthAttachment;
    std::vector<std::string> sampledTextures;
    std::vector<std::string> usedBuffers;
    std::vector<TextureDescriptor> textureDeclarations;
    std::vector<ConstantBufferDescriptor> constantBufferDeclarations;
    std::vector<StructuredBufferDescriptor> structuredBufferDeclarations;
    std::function<void(FramePassSetupContext&)> setup;
    std::function<void(FramePassExecuteContext&)> execute;

    FramePass() = default;
    FramePass(const FramePass&) = default;
    FramePass(FramePass&&) = default;
    FramePass(const FramePassBuilder auto& builder) : FramePass(builder.build()) {}
};

class GE_API FrameGraph
{
public:
    struct Descriptor
    {
        std::string backBufferName;
        std::vector<TextureDescriptor> textures;
        std::vector<ConstantBufferDescriptor> constantBuffers;
        std::vector<StructuredBufferDescriptor> structuredBuffers;
        std::vector<FramePass> passes;
    };

public:
    FrameGraph() = default;
    FrameGraph(const FrameGraph&) = default;
    FrameGraph(FrameGraph&&) = default;

    FrameGraph(const Descriptor&);

    const std::string& backBufferName() const { return m_backBufferName; }
    const std::map<std::string, gfx::Texture::Descriptor>& textureDescriptors() const { return m_textureDescriptors; }
    const std::map<std::string, gfx::Buffer::Descriptor>& constantBufferDescriptors() const { return m_constantBufferDescriptors; }
    const std::set<std::string>& structuredBufferNames() const { return m_structuredBufferNames; }
    const std::vector<FramePass>& passes() const { return m_passes; }

    ~FrameGraph() = default;

private:
    std::vector<FramePass> m_passes;
    std::string m_backBufferName;
    std::map<std::string, gfx::Texture::Descriptor> m_textureDescriptors;
    std::map<std::string, gfx::Buffer::Descriptor> m_constantBufferDescriptors;
    std::set<std::string> m_structuredBufferNames;

public:
    FrameGraph& operator=(const FrameGraph&) = default;
    FrameGraph& operator=(FrameGraph&&) = default;
};

} // namespace GE

#endif

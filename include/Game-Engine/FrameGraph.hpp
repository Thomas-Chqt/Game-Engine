/*
 * ---------------------------------------------------
 * FrameGraph.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef FRAMEGRAPH_HPP
#define FRAMEGRAPH_HPP

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/ParameterBlockPool.hpp>
#include <Graphics/Enums.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Buffer.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace GE
{

struct FramePass;

template<class T>
concept FramePassBuilder = requires(const T& b) {
    { b.build() } -> std::convertible_to<FramePass>;
};

struct AttachmentDescriptor
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

struct ColorAttachmentUsage
{
    std::string name;
    gfx::LoadAction loadAction = gfx::LoadAction::load;
    std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct DepthAttachmentUsage
{
    std::string name;
    gfx::LoadAction loadAction = gfx::LoadAction::clear;
    float clearDepth = 1.0f;
};

struct FramePassSetupContext
{
    std::map<std::string, std::shared_ptr<gfx::Buffer>>& bufferMap;
    std::function<void(const std::string& name, uint32_t size)> resizeStructuredBuffer;
};

struct FramePassContext
{
    gfx::CommandBuffer& commandBuffer;
    gfx::ParameterBlockPool& parameterBlockPool;
    std::map<std::string, std::shared_ptr<gfx::Texture>>& textureMap;
    std::map<std::string, std::shared_ptr<gfx::Buffer>>& bufferMap;

    std::shared_ptr<gfx::ParameterBlockLayout> frameDataBlockLayout;
    std::shared_ptr<gfx::ParameterBlockLayout> materialBlockLayout;
    std::shared_ptr<gfx::GraphicsPipeline> gfxPipeline; // only one for now

    std::pair<uint32_t, uint32_t> renderSize;
};

struct FramePass
{
    std::vector<ColorAttachmentUsage> colorAttachments;
    std::optional<DepthAttachmentUsage> depthAttachment;
    std::vector<std::string> sampledAttachments;
    std::function<void(FramePassSetupContext&)> setup;
    std::function<void(FramePassContext&)> execute;

    FramePass() = default;
    FramePass(const FramePass&) = default;
    FramePass(FramePass&&) = default;
    FramePass(const FramePassBuilder auto& builder) : FramePass(builder.build()) {}
};

class FrameGraph
{
public:
    struct Descriptor
    {
        std::string backBufferName;
        std::vector<AttachmentDescriptor> attachments;
        std::vector<ConstantBufferDescriptor> constantBuffers;
        std::vector<StructuredBufferDescriptor> structuredBuffers;
        std::vector<FramePass> passes;
    };

public:
    FrameGraph() = default;
    FrameGraph(const FrameGraph&) = default;
    FrameGraph(FrameGraph&&) = default;

    FrameGraph(const Descriptor&);

    const std::pair<std::string, gfx::Texture::Descriptor>& backBufferDescriptor() const { return m_backBufferDescriptor; }
    const std::map<std::string, gfx::Texture::Descriptor>& textureDescriptors() const { return m_textureDescriptors; }
    const std::map<std::string, gfx::Buffer::Descriptor>& constantBufferDescriptors() const { return m_constantBufferDescriptors; }
    const std::set<std::string>& structuredBufferNames() const { return m_structuredBufferNames; }
    const std::vector<FramePass>& passes() const { return m_passes; }

    ~FrameGraph() = default;

private:
    std::vector<FramePass> m_passes;
    std::pair<std::string, gfx::Texture::Descriptor> m_backBufferDescriptor;
    std::map<std::string, gfx::Texture::Descriptor> m_textureDescriptors;
    std::map<std::string, gfx::Buffer::Descriptor> m_constantBufferDescriptors;
    std::set<std::string> m_structuredBufferNames;

public:
    FrameGraph& operator=(const FrameGraph&) = default;
    FrameGraph& operator=(FrameGraph&&) = default;
};

} // namespace GE

#endif

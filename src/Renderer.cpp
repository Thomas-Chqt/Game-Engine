/*
 * ---------------------------------------------------
 * Renderer.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/09 15:54:42
 * ---------------------------------------------------
 */

#include "Game-Engine/Renderer.hpp"
#include "Game-Engine/FrameGraph.hpp"
#include "Game-Engine/Mesh.hpp"
#include "TextureTable.hpp"

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/Drawable.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/GraphicsPipeline.hpp>
#include <Graphics/Enums.hpp>
#include <Graphics/Buffer.hpp>

#include <algorithm>
#include <cstdint>
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <ranges>

namespace GE
{

namespace
{

constexpr uint32_t MAX_TEXTURES = 4096;

}

Renderer::Renderer(gfx::Device* device, AssetManager* assetManager, gfx::Surface* surface)
    : m_device(device)
    , m_assetManager(assetManager)
    , m_surface(surface)
{
    assert(m_device);
    assert(m_assetManager);
    assert(m_surface);

    std::shared_ptr<gfx::ParameterBlockLayout> textureTablePBLayout = m_device->newParameterBlockLayout(gfx::ParameterBlockLayout::Descriptor{
        .bindings = {
            gfx::ParameterBlockBinding{ .type = gfx::BindingType::sampler,        .usages = gfx::BindingUsage::fragmentRead },
            gfx::ParameterBlockBinding{ .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::fragmentRead, .count = MAX_TEXTURES },
        }
    });

    m_textureTableBlock = m_device->newParameterBlockPool(gfx::ParameterBlockPool::Descriptor{
        .maxBindingCount = {
            { gfx::BindingType::sampler, 1 },
            { gfx::BindingType::sampledTexture, MAX_TEXTURES },
        },
        .updateAfterBind = true
    })->get(textureTablePBLayout);

    m_textureTableBlock->setBinding(0, std::shared_ptr<gfx::Sampler>(m_device->newSampler(gfx::Sampler::Descriptor{
        .sAddressMode = gfx::SamplerAddressMode::Repeat,
        .tAddressMode = gfx::SamplerAddressMode::Repeat,
        .rAddressMode = gfx::SamplerAddressMode::Repeat,
        .minFilter = gfx::SamplerMinMagFilter::Linear,
        .magFilter = gfx::SamplerMinMagFilter::Linear
    })));
    m_textureTable = std::make_shared<TextureTable>(m_textureTableBlock.get(), 1);
    m_assetManager->attachTextureTable(m_textureTable);

    m_frameDataBlockLayout = m_device->newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::constantBuffer,   .usages = gfx::BindingUsage::vertexRead | gfx::BindingUsage::fragmentRead },
            { .type = gfx::BindingType::structuredBuffer, .usages = gfx::BindingUsage::vertexRead | gfx::BindingUsage::fragmentRead },
            { .type = gfx::BindingType::structuredBuffer, .usages = gfx::BindingUsage::vertexRead | gfx::BindingUsage::fragmentRead }
        }
    });

    m_texturedMaterialPBlockLayout = m_device->newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::structuredBuffer, .usages = gfx::BindingUsage::fragmentRead }
        }
    });

    auto texturedShaderLib = m_device->newShaderLib(SHADER_DIR"/textured.slib");

    m_texturedPipeline = m_device->newGraphicsPipeline(gfx::GraphicsPipeline::Descriptor{
        .vertexLayout = gfx::VertexLayout{
            .stride = sizeof(Vertex),
            .attributes = {
                gfx::VertexAttribute{
                    .format = gfx::VertexAttributeFormat::float3,
                    .offset = offsetof(Vertex, pos)},
                gfx::VertexAttribute{
                    .format = gfx::VertexAttributeFormat::float2,
                    .offset = offsetof(Vertex, uv)},
                gfx::VertexAttribute{
                    .format = gfx::VertexAttributeFormat::float3,
                    .offset = offsetof(Vertex, normal)},
            }
        },
        .vertexShader = &texturedShaderLib->getFunction("vertexMain"),
        .fragmentShader = &texturedShaderLib->getFunction("fragmentMain"),
        .colorAttachmentPxFormats = {gfx::PixelFormat::BGRA8Unorm},
        .depthAttachmentPxFormat = gfx::PixelFormat::Depth32Float,
        .blendOperation = gfx::BlendOperation::blendingOff,
        .cullMode = gfx::CullMode::back,
        .parameterBlockLayouts = {
            textureTablePBLayout,
            m_frameDataBlockLayout,
            m_texturedMaterialPBlockLayout
        }
    });

    for (auto& inFlightData : m_inFlightDatas)
    {
        inFlightData.commandBufferPool = m_device->newCommandBufferPool();
        assert(inFlightData.commandBufferPool);

        inFlightData.parameterBlockPool = m_device->newParameterBlockPool({
            .maxBindingCount = {
                {gfx::BindingType::constantBuffer, 2},
                {gfx::BindingType::structuredBuffer, 3},
            }
        });
        assert(inFlightData.parameterBlockPool);
    }
}

FrameGraphBuilder Renderer::newFrameGraphBuilder()
{
    return {m_textureTable.get(), m_assetManager};
}

void Renderer::renderFrame(const FrameGraph& frameGraph)
{
    ZoneScopedN("Renderer::renderFrame");

    if (cfd.waitedCmdBuffer != nullptr)
    {
        ZoneScopedN("Renderer::wait in-flight frame");
        m_device->waitCommandBuffer(*cfd.waitedCmdBuffer);
        cfd.waitedCmdBuffer = nullptr;
        cfd.commandBufferPool->reset();
        cfd.parameterBlockPool->reset();
    }

    TracyCZoneN(rendererPreparetextures, "Renderer::prepare textures", true);
    std::vector<std::shared_ptr<gfx::Texture>> textureMap(frameGraph.textures.size());
    std::map<gfx::Texture::Descriptor, std::set<std::shared_ptr<gfx::Texture>>> newTextureCache;
    for (uint32_t textureIdx = 0; const auto& texture : frameGraph.textures)
    {
        if (textureIdx == frameGraph.backBuffer && (m_swapchain == nullptr || m_swapchain->drawablesTextureDescriptor() != texture.descriptor))
        {
            ZoneScopedN("Renderer::recreate swapchain");
            gfx::Swapchain::Descriptor swapchainDescriptor = {
                .surface = m_surface,
                .width = texture.descriptor.width,
                .height = texture.descriptor.height,
                .imageCount = 3,
                .drawableCount = maxFrameInFlight,
                .pixelFormat = texture.descriptor.pixelFormat,
                .presentMode = gfx::PresentMode::fifo,
            };
            m_device->waitIdle();
            m_swapchain = m_device->newSwapchain(swapchainDescriptor);
            assert(m_swapchain);
        }
        else if (textureIdx != frameGraph.backBuffer)
        {
            std::shared_ptr<gfx::Texture> gfxTexture;
            auto it = cfd.textureCache.find(texture.descriptor);
            if (it == cfd.textureCache.end() || it->second.empty())
                gfxTexture = m_device->newTexture(texture.descriptor);
            else
                gfxTexture = it->second.extract(it->second.begin()).value();
            textureMap.at(textureIdx) = gfxTexture;
            newTextureCache[texture.descriptor].insert(gfxTexture);
        }
        textureIdx++;
    }
    TracyCZoneEnd(rendererPreparetextures);

    TracyCZoneN(rendererPrepareBuffers, "Renderer::prepare buffers", true);
    std::vector<std::shared_ptr<gfx::Buffer>> bufferMap(frameGraph.buffers.size());
    std::map<gfx::Buffer::Descriptor, std::set<std::shared_ptr<gfx::Buffer>>> newBufferCache;
    for (uint32_t bufferIdx = 0; const auto& buffer : frameGraph.buffers)
    {
        std::shared_ptr<gfx::Buffer> gfxBuffer;
        auto it = cfd.bufferCache.find(buffer.descriptor);
        if (it == cfd.bufferCache.end() || it->second.empty())
            gfxBuffer = m_device->newBuffer(buffer.descriptor);
        else
            gfxBuffer = it->second.extract(it->second.begin()).value();

        const std::span<const std::byte> bufferContent(frameGraph.buffersContent.data() + buffer.offset, buffer.descriptor.size);
        std::memcpy(gfxBuffer->content<std::byte>(), bufferContent.data(), bufferContent.size());

        bufferMap.at(bufferIdx) = gfxBuffer;
        newBufferCache[buffer.descriptor].insert(gfxBuffer);
        bufferIdx++;
    }
    TracyCZoneEnd(rendererPrepareBuffers);

    std::shared_ptr<gfx::CommandBuffer> commandBuffer = cfd.commandBufferPool->get();
    std::shared_ptr<gfx::Drawable> drawable;
    for (auto& framePass : frameGraph.passes)
    {
        ZoneScopedN("Renderer::frame pass");

        if (drawable == nullptr && std::ranges::any_of(framePass.colorAttachments, [&](auto& attachment){return attachment.texture == frameGraph.backBuffer ;}))
        {
            drawable = m_swapchain->nextDrawable();
            if (drawable == nullptr) {
                m_swapchain = nullptr;
                return;
            }
            textureMap.at(frameGraph.backBuffer) = drawable->texture();
        }

        gfx::Framebuffer framebuffer = gfx::Framebuffer{
            .colorAttachments = framePass.colorAttachments
                                | std::views::transform([&](const FrameGraph::Attachment& attachment) {
                                      return gfx::Framebuffer::Attachment{
                                          .loadAction = attachment.loadAction,
                                          .clearColor = attachment.clearColor,
                                          .texture = textureMap.at(attachment.texture)
                                      };
                                  })
                                | std::ranges::to<std::vector>(),
            .depthAttachment = framePass.depthAttachment
                                   ? std::make_optional(gfx::Framebuffer::Attachment{
                                         .loadAction = framePass.depthAttachment->loadAction,
                                         .clearDepth = framePass.depthAttachment->clearDepth,
                                         .texture = textureMap.at(framePass.depthAttachment->texture) })
                                   : std::nullopt
        };

        for (FrameGraph::TextureRef texture : framePass.sampledTextures)
            commandBuffer->addSampledTexture(textureMap.at(texture));

        commandBuffer->beginRenderPass(framebuffer);
        {
            FramePass::ExecuteContext framePassContext = {
                .assetManager = *m_assetManager,
                .commandBuffer = *commandBuffer,
                .parameterBlockPool = *cfd.parameterBlockPool,
                .textureTableBlock = m_textureTableBlock,
                .frameDataBlockLayout = m_frameDataBlockLayout,
                .texture = [&](FrameGraph::TextureRef ref){ return textureMap.at(ref); },
                .buffer = [&](FrameGraph::BufferRef ref){ return bufferMap.at(ref); },
                .texturedMaterialPBlockLayout = m_texturedMaterialPBlockLayout,
                .texturedPipeline = m_texturedPipeline
            };
            framePass.execute(framePassContext);
        }
        commandBuffer->endRenderPass();
    }

    if (drawable)
        commandBuffer->presentDrawable(drawable);

    m_device->submitCommandBuffers(commandBuffer);
    cfd.waitedCmdBuffer = commandBuffer.get();

    cfd.textureCache = std::move(newTextureCache);
    cfd.bufferCache = std::move(newBufferCache);
    m_frameIdx = (m_frameIdx + 1) % maxFrameInFlight;
}

Renderer::~Renderer()
{
    m_device->waitIdle();
}

} // namespace GE

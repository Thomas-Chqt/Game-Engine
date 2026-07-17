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
#include <Graphics/PassDescriptor.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/GraphicsPipeline.hpp>
#include <Graphics/Enums.hpp>
#include <Graphics/Buffer.hpp>

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#include <gfx_tracy/gfx_tracy.hpp>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <ranges>
#include <algorithm>
#include <cstdint>

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
    , m_tracyGfxCtx(TracyGFXContext(*m_device))
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
        .colorAttachmentPxFormats = {gfx::PixelFormat::BGRA8Unorm, gfx::PixelFormat::RG32Uint},
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

void Renderer::renderFrame(FrameGraph frameGraph)
{
    ZoneScoped;

    if (cfd.waitedCmdBuffer != nullptr)
    {
        ZoneScopedN("Renderer::wait_in-flight_frame");
        m_device->waitCommandBuffer(*cfd.waitedCmdBuffer);
        TracyGFXCollect(*m_device, m_tracyGfxCtx);
        cfd.waitedCmdBuffer = nullptr;

        for (PendingReadback& readback : cfd.pendingReadbacks)
        {
            assert(readback.buffer);
            assert(readback.request);
            assert(readback.buffer->storageMode() == gfx::ResourceStorageMode::hostVisible);
            readback.request->resolve(std::span<std::byte>(readback.buffer->content<std::byte>(), readback.buffer->size()));
        }

        cfd.commandBufferPool->reset();
        cfd.parameterBlockPool->reset();
    }

    TracyCZoneN(rendererPreparetextures, "Renderer::prepare_textures", true);
    std::vector<std::shared_ptr<gfx::Texture>> textureMap(frameGraph.textures.size());
    std::map<gfx::Texture::Descriptor, std::set<std::shared_ptr<gfx::Texture>>> newTextureCache;
    for (uint32_t textureIdx = 0; textureIdx < frameGraph.textures.size(); textureIdx++)
    {
        const FrameGraph::Texture& texture = frameGraph.textures.at(textureIdx);
        if (textureIdx == frameGraph.backBuffer && (m_swapchain == nullptr || m_swapchain->drawablesTextureDescriptor() != texture.descriptor))
        {
            ZoneScopedN("Renderer::recreate_swapchain");
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
    }
    TracyCZoneEnd(rendererPreparetextures);

    TracyCZoneN(rendererPrepareBuffers, "Renderer::prepare_buffers", true);
    std::vector<std::shared_ptr<gfx::Buffer>> bufferMap(frameGraph.buffers.size());
    std::map<gfx::Buffer::Descriptor, std::set<std::shared_ptr<gfx::Buffer>>> newBufferCache;
    for (uint32_t bufferIdx = 0; bufferIdx < frameGraph.buffers.size(); bufferIdx++)
    {
        const FrameGraph::Buffer& buffer = frameGraph.buffers.at(bufferIdx);
        std::shared_ptr<gfx::Buffer> gfxBuffer;
        auto it = cfd.bufferCache.find(buffer.descriptor);
        if (it == cfd.bufferCache.end() || it->second.empty())
            gfxBuffer = m_device->newBuffer(buffer.descriptor);
        else
            gfxBuffer = it->second.extract(it->second.begin()).value();

        if (buffer.offset.has_value())
        {
            assert(*buffer.offset + buffer.descriptor.size <= frameGraph.buffersContent.size());
            assert(gfxBuffer->storageMode() == gfx::ResourceStorageMode::hostVisible);
            const std::span<const std::byte> bufferContent(frameGraph.buffersContent.data() + *buffer.offset, buffer.descriptor.size);
            std::memcpy(gfxBuffer->content<std::byte>(), bufferContent.data(), bufferContent.size());
        }

        bufferMap.at(bufferIdx) = gfxBuffer;
        newBufferCache[buffer.descriptor].insert(gfxBuffer);
    }
    TracyCZoneEnd(rendererPrepareBuffers);

    TracyCZoneN(rendererPrepareReadbacks, "Renderer::prepare_readbacks", true);
    cfd.pendingReadbacks.clear();
    cfd.pendingReadbacks.reserve(frameGraph.readbacks.size());
    for (std::unique_ptr<FrameGraph::ReadbackBase>& readback : frameGraph.readbacks)
    {
        assert(readback->buffer < bufferMap.size());
        assert(bufferMap.at(readback->buffer));
        assert(bufferMap.at(readback->buffer)->storageMode() == gfx::ResourceStorageMode::hostVisible);
        cfd.pendingReadbacks.push_back(PendingReadback{
            .buffer = bufferMap.at(readback->buffer),
            .request = std::move(readback)
        });
    }
    TracyCZoneEnd(rendererPrepareReadbacks);

    std::shared_ptr<gfx::CommandBuffer> commandBuffer = cfd.commandBufferPool->get();
    std::shared_ptr<gfx::Drawable> drawable;
    for (auto& framePass : frameGraph.passes)
    {
        ZoneScopedN("Renderer::frame_pass");

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

        if (framePass.kind == FramePass::Kind::blit)
        {
            assert(framePass.colorAttachments.empty());
            assert(framePass.depthAttachment.has_value() == false);
            assert(framePass.sampledTextures.empty());
            assert(std::ranges::all_of(framePass.copySourceBuffers, [&](auto bufferRef) { return bufferRef < bufferMap.size(); }));
            assert(std::ranges::all_of(framePass.copyDestinationBuffers, [&](auto bufferRef) { return bufferRef < bufferMap.size(); }));
            assert(std::ranges::all_of(framePass.copySourceTextures, [&](auto textureRef) { return textureRef < textureMap.size(); }));
            assert(std::ranges::all_of(framePass.copyDestinationTextures, [&](auto textureRef) { return textureRef < textureMap.size(); }));

            auto blitPassDescriptor = m_device->newBlitPassDescriptor();
            TracyGFXZone(m_tracyGfxCtx, *blitPassDescriptor, "blitPass");
            commandBuffer->beginBlitPass(*blitPassDescriptor);
            framePass.execute(framePassContext);
            commandBuffer->endBlitPass();
            continue;
        }

        if (drawable == nullptr && std::ranges::any_of(framePass.colorAttachments, [&](auto& attachment){return attachment.texture == frameGraph.backBuffer ;}))
        {
            drawable = m_swapchain->nextDrawable();
            if (drawable == nullptr) {
                m_swapchain = nullptr;
                return;
            }
            textureMap.at(frameGraph.backBuffer) = drawable->texture();
        }

        for (FrameGraph::TextureRef texture : framePass.sampledTextures)
            commandBuffer->addSampledTexture(textureMap.at(texture));

        auto colorAttachments = framePass.colorAttachments | std::views::transform([&](const FrameGraph::Attachment& attachment) {
            return gfx::RenderPassDescriptor::Attachment{
                .loadAction = attachment.loadAction,
                .clearValue = attachment.clearValue,
                .texture = textureMap.at(attachment.texture)
            };
        });

        auto depthAttachment = framePass.depthAttachment.transform([&](const FrameGraph::Attachment& attachment) {
            return gfx::RenderPassDescriptor::Attachment{
                .loadAction = attachment.loadAction,
                .clearValue = attachment.clearValue,
                .texture = textureMap.at(attachment.texture)
            };
        });

        auto renderPassDescriptror = m_device->newRenderPassDescriptor();
        renderPassDescriptror->setColorAttachments(std::move(colorAttachments) | std::ranges::to<std::vector>());
        renderPassDescriptror->setDepthAttachment(std::move(depthAttachment));

        TracyGFXZone(m_tracyGfxCtx, *renderPassDescriptror, "renderPass");
        commandBuffer->beginRenderPass(*renderPassDescriptror);
        framePass.execute(framePassContext);
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

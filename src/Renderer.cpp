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

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/Drawable.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/GraphicsPipeline.hpp>
#include <Graphics/Enums.hpp>
#include <Graphics/Buffer.hpp>

#include <cassert>
#include <cstddef>
#include <cstring>
#include <map>
#include <memory>
#include <optional>
#include <print>
#include <string>
#include <utility>
#include <vector>
#include <ranges>

namespace GE
{

Renderer::Renderer(gfx::Device* device, gfx::Surface* surface)
    : m_device(device), m_surface(surface)
{
    m_frameDataBlockLayout = m_device->newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::constantBuffer,   .usages = gfx::BindingUsage::vertexRead | gfx::BindingUsage::fragmentRead },
            { .type = gfx::BindingType::structuredBuffer, .usages = gfx::BindingUsage::vertexRead | gfx::BindingUsage::fragmentRead },
            { .type = gfx::BindingType::structuredBuffer, .usages = gfx::BindingUsage::vertexRead | gfx::BindingUsage::fragmentRead }
        }
    });
    m_materialBlockLayout = m_device->newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::constantBuffer, .usages = gfx::BindingUsage::vertexRead }
        }
    });

    auto shaderLib = m_device->newShaderLib(SHADER_DIR"/flat_color.slib");

    m_gfxPipeline = m_device->newGraphicsPipeline(gfx::GraphicsPipeline::Descriptor{
        .vertexLayout = gfx::VertexLayout{
            .stride = sizeof(Vertex),
            .attributes = {
                gfx::VertexAttribute{
                    .format = gfx::VertexAttributeFormat::float3,
                    .offset = offsetof(Vertex, pos)},
                gfx::VertexAttribute{
                    .format = gfx::VertexAttributeFormat::float3,
                    .offset = offsetof(Vertex, normal)},
            }
        },
        .vertexShader = &shaderLib->getFunction("vertexMain"),
        .fragmentShader = &shaderLib->getFunction("fragmentMain"),
        .colorAttachmentPxFormats = {gfx::PixelFormat::BGRA8Unorm},
        .depthAttachmentPxFormat = gfx::PixelFormat::Depth32Float,
        .blendOperation = gfx::BlendOperation::blendingOff,
        .cullMode = gfx::CullMode::back,
        .parameterBlockLayouts = {
            m_frameDataBlockLayout,
            m_materialBlockLayout
        }
    });

    for (auto& inFlightData : m_inFlightDatas)
    {
        inFlightData.commandBufferPool = m_device->newCommandBufferPool();
        assert(inFlightData.commandBufferPool);

        inFlightData.parameterBlockPool = m_device->newParameterBlockPool({
            .maxBindingCount = {
                {gfx::BindingType::constantBuffer, 2},
                {gfx::BindingType::structuredBuffer, 2},
            }
        });
        assert(inFlightData.parameterBlockPool);
    }

}

void Renderer::renderFrame(const FrameGraph& frameGraph)
{
    if (cfd.waitedCmdBuffer != nullptr)
    {
        m_device->waitCommandBuffer(*cfd.waitedCmdBuffer);
        cfd.waitedCmdBuffer = nullptr;
        cfd.commandBufferPool->reset();
        cfd.parameterBlockPool->reset();
    }

    std::map<std::string, std::shared_ptr<gfx::Texture>> textureMap;
    std::map<gfx::Texture::Descriptor, std::set<std::shared_ptr<gfx::Texture>>> newTextureCache;
    for (auto& [textureName, textureDescriptor] : frameGraph.textureDescriptors())
    {
        if (textureName == frameGraph.backBufferName() && (m_swapchain == nullptr || m_swapchain->drawablesTextureDescriptor() != textureDescriptor))
        {
            gfx::Swapchain::Descriptor swapchainDescriptor = {
                .surface = m_surface,
                .width = textureDescriptor.width,
                .height = textureDescriptor.height,
                .imageCount = 3,
                .drawableCount = maxFrameInFlight,
                .pixelFormat = textureDescriptor.pixelFormat,
                .presentMode = gfx::PresentMode::fifo,
            };
            // std::println("recreating swapchain with size w:{}, h:{}", swapchainDescriptor.width, swapchainDescriptor.height);
            m_device->waitIdle();
            m_swapchain = m_device->newSwapchain(swapchainDescriptor);
            assert(m_swapchain);
        }
        else
        {
            std::shared_ptr<gfx::Texture> texture;
            auto it = cfd.textureCache.find(textureDescriptor);
            if (it == cfd.textureCache.end() || it->second.empty())
                texture = m_device->newTexture(textureDescriptor);
            else
                texture = it->second.extract(it->second.begin()).value();
            auto [_, inserted] = textureMap.insert(std::make_pair(textureName, texture));
            assert(inserted);
            newTextureCache[textureDescriptor].insert(texture);
        }
    }

    std::map<std::string, std::shared_ptr<gfx::Buffer>> bufferMap;
    std::map<gfx::Buffer::Descriptor, std::set<std::shared_ptr<gfx::Buffer>>> newBufferCache;
    for (auto& [bufferName, bufferDescriptor] : frameGraph.constantBufferDescriptors())
    {
        std::shared_ptr<gfx::Buffer> buffer;
        auto it = cfd.bufferCache.find(bufferDescriptor);
        if (it == cfd.bufferCache.end() || it->second.empty())
            buffer = m_device->newBuffer(bufferDescriptor);
        else
            buffer = it->second.extract(it->second.begin()).value();
        auto [_, inserted] = bufferMap.insert(std::make_pair(bufferName, buffer));
        assert(inserted);
        newBufferCache[bufferDescriptor].insert(buffer);
    }

    auto setStructuredBufferContent = [&](const std::string& bufferName, const void* data, uint32_t size) {
        if (data == nullptr || size == 0)
            return;
        gfx::Buffer::Descriptor bufferDescriptor = gfx::Buffer::Descriptor{
            .size = size,
            .usages = gfx::BufferUsage::structuredBuffer,
            .storageMode = gfx::ResourceStorageMode::hostVisible
        };
        std::shared_ptr<gfx::Buffer> buffer;
        auto it = cfd.bufferCache.find(bufferDescriptor);
        if (it == cfd.bufferCache.end() || it->second.empty())
            buffer = m_device->newBuffer(bufferDescriptor);
        else
            buffer = it->second.extract(it->second.begin()).value();
        auto [_, inserted] = bufferMap.insert(std::make_pair(bufferName, buffer));
        assert(inserted);
        newBufferCache[bufferDescriptor].insert(buffer);
        std::memcpy(buffer->content<std::byte>(), data, size);
    };

    std::shared_ptr<gfx::CommandBuffer> commandBuffer = cfd.commandBufferPool->get();
    std::shared_ptr<gfx::Drawable> drawable;
    for (auto& framePass : frameGraph.passes())
    {
        if (drawable == nullptr /* TODO check if pass uses swapchain image */)
        {
            drawable = m_swapchain->nextDrawable();
            if (drawable == nullptr)
                return;
            textureMap[frameGraph.backBufferName()] = drawable->texture();
        }

        if (framePass.setup)
        {
            FramePassSetupContext setupContext = {
                .textureMap = textureMap,
                .constantBuffers = bufferMap,
                .setStructuredBufferContent = setStructuredBufferContent,
            };
            framePass.setup(setupContext);
        }

        gfx::Framebuffer framebuffer = gfx::Framebuffer{
            .colorAttachments = framePass.colorAttachments
                                | std::views::transform([&](const AttachmentDescriptor& attachment) {
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

        for (auto& textureName : framePass.sampledTextures)
            commandBuffer->addSampledTexture(textureMap.at(textureName));

        commandBuffer->beginRenderPass(framebuffer);
        {
            FramePassExecuteContext framePassContext = {
                .commandBuffer = *commandBuffer,
                .parameterBlockPool = *cfd.parameterBlockPool,
                .textureMap = textureMap,
                .bufferMap = bufferMap,
                .frameDataBlockLayout = m_frameDataBlockLayout,
                .materialBlockLayout = m_materialBlockLayout,
                .gfxPipeline = m_gfxPipeline
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

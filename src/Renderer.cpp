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

#include "shaders/FrameData.slang"
#include "shaders/Light.slang"
#include "shaders/flat_color.slang"

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/Drawable.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/GraphicsPipeline.hpp>
#include <Graphics/Enums.hpp>
#include <Graphics/Buffer.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
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

    const auto& backBufferDesc = frameGraph.textureDescriptors().at(frameGraph.backBufferName());

    if (m_swapchain == nullptr || m_swapchain->drawablesTextureDescriptor() != backBufferDesc)
    {
        gfx::Swapchain::Descriptor swapchainDescriptor = {
            .surface = m_surface,
            .width = backBufferDesc.width,
            .height = backBufferDesc.height,
            .imageCount = 3,
            .drawableCount = maxFrameInFlight,
            .pixelFormat = backBufferDesc.pixelFormat,
            .presentMode = gfx::PresentMode::fifo,
        };
        // std::println("recreating swapchain with size w:{}, h:{}", swapchainDescriptor.width, swapchainDescriptor.height);
        m_device->waitIdle();
        m_swapchain = m_device->newSwapchain(swapchainDescriptor);
        assert(m_swapchain);
    }
    assert(m_swapchain->drawablesTextureDescriptor() == backBufferDesc);

    // Create/reuse transient textures (skip back buffer)
    std::map<std::string, std::shared_ptr<gfx::Texture>> textureMap;
    std::set<std::pair<gfx::Texture::Descriptor, std::shared_ptr<gfx::Texture>>> usedTextures;

    for (auto& [textureName, textureDescriptor] : frameGraph.textureDescriptors())
    {
        if (textureName == frameGraph.backBufferName())
            continue;

        std::shared_ptr<gfx::Texture> texture;
        auto it = std::ranges::find_if(cfd.transientTextures, [&](auto& element) -> bool { return element.first == textureDescriptor; });
        if (it != cfd.transientTextures.end())
            texture = cfd.transientTextures.extract(it).value().second;
        else
            texture = m_device->newTexture(textureDescriptor);
        textureMap.insert(std::make_pair(textureName, texture));
        usedTextures.insert(std::make_pair(textureDescriptor, texture));
    }
    cfd.transientTextures = std::move(usedTextures);

    // Create/reuse constant buffers
    for (auto& [bufferName, bufferDescriptor] : frameGraph.constantBufferDescriptors())
    {
        auto it = cfd.constantBuffers.find(bufferName);
        if (it == cfd.constantBuffers.end() || it->second->size() != bufferDescriptor.size)
        {
            std::shared_ptr<gfx::Buffer> newBuffer = m_device->newBuffer(bufferDescriptor);
            cfd.constantBuffers[bufferName] = newBuffer;
        }
    }

    // Callback to set structured buffer size and content in one call
    auto setStructuredBufferContent = [&](const std::string& name, const void* data, uint32_t size) {
        if (!frameGraph.structuredBufferNames().contains(name))
            return;
        uint32_t allocSize = std::max(size, (uint32_t)1);
        auto it = cfd.structuredBuffers.find(name);
        if (it == cfd.structuredBuffers.end() || it->second->size() < allocSize)
        {
            std::shared_ptr<gfx::Buffer> newBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
                .size = allocSize,
                .usages = gfx::BufferUsage::structuredBuffer,
                .storageMode = gfx::ResourceStorageMode::hostVisible
            });
            cfd.structuredBuffers[name] = newBuffer;
        }
        if (data != nullptr && size > 0)
            std::memcpy(cfd.structuredBuffers[name]->content<char>(), data, size);
    };

    std::shared_ptr<gfx::CommandBuffer> commandBuffer = cfd.commandBufferPool->get();
    std::shared_ptr<gfx::Drawable> drawable;
    for (auto& framePass : frameGraph.passes())
    {
        // Run setup phase if present
        if (framePass.setup)
        {
            FramePassSetupContext setupContext = {
                .constantBuffers = cfd.constantBuffers,
                .setStructuredBufferContent = setStructuredBufferContent,
            };
            framePass.setup(setupContext);
        }

        // Build combined buffer map for execute phase (after setup may have modified structured buffers)
        std::map<std::string, std::shared_ptr<gfx::Buffer>> bufferMap;
        for (auto& [name, buffer] : cfd.constantBuffers)
            bufferMap[name] = buffer;
        for (auto& [name, buffer] : cfd.structuredBuffers)
            bufferMap[name] = buffer;

        if (drawable == nullptr /* TODO check if pass use swapchain image */)
        {
            drawable = m_swapchain->nextDrawable();
            if (drawable == nullptr)
                return;
            textureMap[frameGraph.backBufferName()] = drawable->texture();
        }

        gfx::Framebuffer framebuffer = gfx::Framebuffer{
            .colorAttachments = framePass.colorAttachments
                                | std::views::transform([&](const AttachmentUsage& attachmentUsage) {
                                      return gfx::Framebuffer::Attachment{
                                          .loadAction = attachmentUsage.loadAction,
                                          .clearColor = attachmentUsage.clearColor,
                                          .texture = textureMap.at(attachmentUsage.name)
                                      };
                                  })
                                | std::ranges::to<std::vector>(),
            .depthAttachment = framePass.depthAttachment
                                   ? std::make_optional(gfx::Framebuffer::Attachment{
                                         .loadAction = framePass.depthAttachment->loadAction,
                                         .clearDepth = framePass.depthAttachment->clearDepth,
                                         .texture = textureMap.at(framePass.depthAttachment->name) })
                                   : std::nullopt
        };

        for (auto attachmentName : framePass.sampledAttachments)
            commandBuffer->addSampledTexture(textureMap.at(attachmentName));

        commandBuffer->beginRenderPass(framebuffer);
        {
            FramePassContext framePassContext = {
                .commandBuffer = *commandBuffer,
                .parameterBlockPool = *cfd.parameterBlockPool,
                .textureMap = textureMap,
                .bufferMap = bufferMap,
                .frameDataBlockLayout = m_frameDataBlockLayout,
                .materialBlockLayout = m_materialBlockLayout,
                .gfxPipeline = m_gfxPipeline,
                .renderSize = {
                    framebuffer.colorAttachments[0].texture->width(),
                    framebuffer.colorAttachments[0].texture->height()
                }
            };
            framePass.execute(framePassContext);
        }
        commandBuffer->endRenderPass();
    }

    if (drawable)
        commandBuffer->presentDrawable(drawable);

    m_device->submitCommandBuffers(commandBuffer);
    cfd.waitedCmdBuffer = commandBuffer.get();

    m_frameIdx = (m_frameIdx + 1) % maxFrameInFlight;
}

Renderer::~Renderer()
{
    m_device->waitIdle();
}

} // namespace GE

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

        inFlightData.frameDataBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
            .size = sizeof(shader::FrameData),
            .usages = gfx::BufferUsage::constantBuffer,
            .storageMode = gfx::ResourceStorageMode::hostVisible
        });
        assert(inFlightData.frameDataBuffer);

        inFlightData.directionalLightsBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
            .size = sizeof(shader::DirectionalLight) * 10,
            .usages = gfx::BufferUsage::structuredBuffer,
            .storageMode = gfx::ResourceStorageMode::hostVisible
        });
        assert(inFlightData.directionalLightsBuffer);

        inFlightData.pointLightsBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
            .size = sizeof(shader::PointLight) * 10,
            .usages = gfx::BufferUsage::structuredBuffer,
            .storageMode = gfx::ResourceStorageMode::hostVisible
        });
        assert(inFlightData.pointLightsBuffer);

        inFlightData.materialBuffer = m_device->newBuffer(gfx::Buffer::Descriptor{
            .size = sizeof(shader::flat_color::Material),
            .usages = gfx::BufferUsage::structuredBuffer,
            .storageMode = gfx::ResourceStorageMode::hostVisible
        });
        assert(inFlightData.materialBuffer);
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

    if (m_swapchain == nullptr || m_swapchain->drawablesTextureDescriptor() != frameGraph.backBufferDescriptor().second)
    {
        gfx::Swapchain::Descriptor swapchainDescriptor = {
            .surface = m_surface,
            .width = frameGraph.backBufferDescriptor().second.width,
            .height = frameGraph.backBufferDescriptor().second.height,
            .imageCount = 3,
            .drawableCount = maxFrameInFlight,
            .pixelFormat = frameGraph.backBufferDescriptor().second.pixelFormat,
            .presentMode = gfx::PresentMode::fifo,
        };
        // std::println("recreating swapchain with size w:{}, h:{}", swapchainDescriptor.width, swapchainDescriptor.height);
        m_device->waitIdle();
        m_swapchain = m_device->newSwapchain(swapchainDescriptor);
        assert(m_swapchain);
    }
    assert(m_swapchain->drawablesTextureDescriptor() == frameGraph.backBufferDescriptor().second);

    std::map<std::string, std::shared_ptr<gfx::Texture>> textureMap;
    std::set<std::pair<gfx::Texture::Descriptor, std::shared_ptr<gfx::Texture>>> usedTextures;

    for (auto& [textureName, textureDescriptor] : frameGraph.textureDescriptors())
    {
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

    std::shared_ptr<gfx::CommandBuffer> commandBuffer = cfd.commandBufferPool->get();
    std::shared_ptr<gfx::Drawable> drawable;
    for (auto& framePass : frameGraph.passes())
    {
        if (drawable == nullptr /* TODO check if pass use swapchain image */)
        {
            drawable = m_swapchain->nextDrawable();
            if (drawable == nullptr)
                return;
            textureMap[frameGraph.backBufferDescriptor().first] = drawable->texture();
        }

        gfx::Framebuffer framebuffer = gfx::Framebuffer{
            .colorAttachments = framePass.colorAttachments
                                | std::views::transform([&](const AttachmentDescriptor& attachmentDesc) {
                                      return gfx::Framebuffer::Attachment{
                                          .loadAction = attachmentDesc.loadAction,
                                          .clearColor = attachmentDesc.clearColor,
                                          .texture = textureMap.at(attachmentDesc.name)
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
            commandBuffer->addSampledTexture(textureMap[attachmentName]);

        commandBuffer->beginRenderPass(framebuffer);
        {
            FramePassContext framePassContext = {
                .commandBuffer = *commandBuffer,
                .parameterBlockPool = *cfd.parameterBlockPool,
                .textureMap = textureMap,
                .frameDataBuffer = cfd.frameDataBuffer,
                .directionalLightsBuffer = cfd.directionalLightsBuffer,
                .pointLightsBuffer = cfd.pointLightsBuffer,
                .materialBuffer = cfd.materialBuffer,
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

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

    // ------------- flat color ------------- //
    m_flatColorMaterialPBlockLayout = m_device->newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::constantBuffer, .usages = gfx::BindingUsage::fragmentRead }
        }
    });

    auto flatColorShaderLib = m_device->newShaderLib(SHADER_DIR"/flat_color.slib");

    m_flatColorPipeline = m_device->newGraphicsPipeline(gfx::GraphicsPipeline::Descriptor{
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
        .vertexShader = &flatColorShaderLib->getFunction("vertexMain"),
        .fragmentShader = &flatColorShaderLib->getFunction("fragmentMain"),
        .colorAttachmentPxFormats = {gfx::PixelFormat::BGRA8Unorm},
        .depthAttachmentPxFormat = gfx::PixelFormat::Depth32Float,
        .blendOperation = gfx::BlendOperation::blendingOff,
        .cullMode = gfx::CullMode::back,
        .parameterBlockLayouts = {
            m_frameDataBlockLayout,
            m_flatColorMaterialPBlockLayout
        }
    });

    // ------------- textured ------------- //

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
        else if (textureName != frameGraph.backBufferName())
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
        if (size == 0)
            return;
        auto bufferDescriptor = gfx::Buffer::Descriptor{
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
        if (data != nullptr)
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
                .assetManager = *m_assetManager,
                .textureTable = *m_textureTable,
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
                .assetManager = *m_assetManager,
                .textureTable = *m_textureTable,

                .commandBuffer = *commandBuffer,
                .parameterBlockPool = *cfd.parameterBlockPool,

                .textureMap = textureMap,
                .bufferMap = bufferMap,

                .textureTableBlock = m_textureTableBlock,
                .frameDataBlockLayout = m_frameDataBlockLayout,

                .flatColorMaterialPBlockLayout = m_flatColorMaterialPBlockLayout,
                .flatColorPipeline = m_flatColorPipeline,

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

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

#include <Graphics/ComputePipeline.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/Drawable.hpp>
#include <Graphics/PassDescriptor.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/GraphicsPipeline.hpp>
#include <Graphics/Enums.hpp>
#include <Graphics/Buffer.hpp>

#include <stb_image/stb_image.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#include <gfx_tracy/gfx_tracy.hpp>

#include <bit>
#include <format>
#include <stdexcept>
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

uint16_t floatToSaturatedHalf(float value)
{
    constexpr float maxFiniteHalf = 65504.0f;
    value = std::clamp(value, -maxFiniteHalf, maxFiniteHalf);

    const uint32_t bits = std::bit_cast<uint32_t>(value);
    const uint32_t sign = (bits >> 16) & 0x8000u;
    const int32_t exponent = static_cast<int32_t>((bits >> 23) & 0xFFu) - 127 + 15;
    const uint32_t mantissa = bits & 0x7FFFFFu;

    if (exponent >= 31)
        return static_cast<uint16_t>(sign | 0x7BFFu);
    if (exponent <= 0)
        return static_cast<uint16_t>(sign); // denormals flushed to zero
    return static_cast<uint16_t>(sign | (static_cast<uint32_t>(exponent) << 10) | (mantissa >> 13));
}

std::shared_ptr<gfx::Texture> createEnvironmentCubemap(gfx::Device& device, gfx::ShaderLib& pbrPrecomputeSlib)
{
    constexpr const char* environmentPath = RESOURCE_DIR"/environment.hdr";

    int width = 0;
    int height = 0;
    std::unique_ptr<float, decltype(&stbi_image_free)> pixels(
        stbi_loadf(environmentPath, &width, &height, nullptr, STBI_rgb_alpha),
        stbi_image_free
    );
    if (!pixels)
    {
        const char* failureReason = stbi_failure_reason();
        throw std::runtime_error(std::format(
            "failed to load HDR image: {}",
            failureReason != nullptr ? failureReason : "unknown error"
        ));
    }
    if (width != height * 2 || height < 2)
        throw std::runtime_error("environment HDR must be a 2:1 equirectangular image");

    const auto faceSize = static_cast<uint32_t>(height / 2);
    const size_t componentCount = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    const size_t uploadSize =
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        gfx::pixelFormatSize(gfx::PixelFormat::RGBA16_float);

    std::shared_ptr<gfx::Buffer> uploadBuffer = device.newBuffer({
        .size = uploadSize,
        .usages = gfx::BufferUsage::copySource,
        .storageMode = gfx::ResourceStorageMode::hostVisible
    });
    assert(uploadBuffer);
    auto* halfPixels = uploadBuffer->content<uint16_t>();
    for (size_t i = 0; i < componentCount; ++i)
        halfPixels[i] = floatToSaturatedHalf(pixels.get()[i]); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    std::shared_ptr<gfx::Texture> equirectangularTexture = device.newTexture({
        .type = gfx::TextureType::texture2d,
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .mipLevelCount = 1,
        .arrayLayerCount = 1,
        .pixelFormat = gfx::PixelFormat::RGBA16_float,
        .usages = gfx::TextureUsage::copyDestination | gfx::TextureUsage::shaderRead,
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
    assert(equirectangularTexture);

    std::shared_ptr<gfx::Texture> cubemap = device.newTexture({
        .type = gfx::TextureType::textureCube,
        .width = faceSize,
        .height = faceSize,
        .mipLevelCount = static_cast<uint32_t>(std::bit_width(faceSize)),
        .arrayLayerCount = 1,
        .pixelFormat = gfx::PixelFormat::RGBA16_float,
        .usages =
            gfx::TextureUsage::copySource |
            gfx::TextureUsage::copyDestination |
            gfx::TextureUsage::shaderRead |
            gfx::TextureUsage::shaderWrite,
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
    assert(cubemap);

    std::shared_ptr<gfx::Texture> writableFaces = device.newTextureView(cubemap, {
        .type = gfx::TextureType::texture2dArray,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 6
    });
    assert(writableFaces);

    std::shared_ptr<gfx::Sampler> sampler = device.newSampler({
        .sAddressMode = gfx::SamplerAddressMode::Repeat,
        .tAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .rAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .minFilter = gfx::SamplerMinMagFilter::Linear,
        .magFilter = gfx::SamplerMinMagFilter::Linear
    });
    assert(sampler);

    std::shared_ptr<gfx::ParameterBlockLayout> parameterBlockLayout = device.newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::computeRead },
            { .type = gfx::BindingType::sampler, .usages = gfx::BindingUsage::computeRead },
            { .type = gfx::BindingType::storageTexture, .usages = gfx::BindingUsage::computeWrite }
        }
    });
    assert(parameterBlockLayout);

    std::shared_ptr<gfx::ComputePipeline> conversionPipeline = device.newComputePipeline({
        .computeShader = &pbrPrecomputeSlib.getFunction("equirectangularToCubemap"),
        .threadsPerThreadgroupX = 8,
        .threadsPerThreadgroupY = 8,
        .threadsPerThreadgroupZ = 1,
        .parameterBlockLayouts = { parameterBlockLayout }
    });
    assert(conversionPipeline);

    std::unique_ptr<gfx::ParameterBlockPool> parameterBlockPool = device.newParameterBlockPool({
        .maxBindingCount = {
            { gfx::BindingType::sampledTexture, 1 },
            { gfx::BindingType::storageTexture, 1 },
            { gfx::BindingType::sampler, 1 }
        }
    });
    assert(parameterBlockPool);

    std::shared_ptr<gfx::ParameterBlock> parameters = parameterBlockPool->get(parameterBlockLayout);
    parameters->setBinding(0, equirectangularTexture);
    parameters->setBinding(1, sampler);
    parameters->setBinding(2, writableFaces);

    std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = device.newCommandBufferPool();
    assert(commandBufferPool);
    std::shared_ptr<gfx::CommandBuffer> commandBuffer = commandBufferPool->get();

    std::unique_ptr<gfx::BlitPassDescriptor> blitPass = device.newBlitPassDescriptor();
    commandBuffer->beginBlitPass(*blitPass);
    commandBuffer->copyBufferToTexture(uploadBuffer, equirectangularTexture);
    commandBuffer->endBlitPass();

    std::unique_ptr<gfx::ComputePassDescriptor> computePass = device.newComputePassDescriptor();
    commandBuffer->beginComputePass(*computePass);
    commandBuffer->usePipeline(conversionPipeline);
    commandBuffer->setParameterBlock(parameters, 0);
    commandBuffer->dispatchThreadgroups((faceSize + 7) / 8, (faceSize + 7) / 8, 6);
    commandBuffer->endComputePass();

    std::unique_ptr<gfx::BlitPassDescriptor> mipmapPass = device.newBlitPassDescriptor();
    commandBuffer->beginBlitPass(*mipmapPass);
    commandBuffer->generateMipmaps(cubemap);
    commandBuffer->endBlitPass();

    device.submitCommandBuffers(commandBuffer);

    return cubemap;
}

std::shared_ptr<gfx::Texture> createIrradianceCubemap(
    gfx::Device& device,
    gfx::ShaderLib& pbrPrecomputeSlib,
    const std::shared_ptr<gfx::Texture>& environmentCubemap
)
{
    assert(environmentCubemap);

    constexpr uint32_t irradianceFaceSize = 32;

    std::shared_ptr<gfx::Texture> irradianceCubemap = device.newTexture({
        .type = gfx::TextureType::textureCube,
        .width = irradianceFaceSize,
        .height = irradianceFaceSize,
        .mipLevelCount = 1,
        .arrayLayerCount = 1,
        .pixelFormat = gfx::PixelFormat::RGBA16_float,
        .usages = gfx::TextureUsage::shaderRead | gfx::TextureUsage::shaderWrite,
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
    assert(irradianceCubemap);

    std::shared_ptr<gfx::Texture> writableFaces = device.newTextureView(irradianceCubemap, {
        .type = gfx::TextureType::texture2dArray,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 6
    });
    assert(writableFaces);

    std::shared_ptr<gfx::Sampler> sampler = device.newSampler({
        .sAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .tAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .rAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .minFilter = gfx::SamplerMinMagFilter::Linear,
        .magFilter = gfx::SamplerMinMagFilter::Linear,
        .mipFilter = gfx::SamplerMipFilter::Linear
    });
    assert(sampler);

    std::shared_ptr<gfx::ParameterBlockLayout> parameterBlockLayout = device.newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::computeRead },
            { .type = gfx::BindingType::sampler, .usages = gfx::BindingUsage::computeRead },
            { .type = gfx::BindingType::storageTexture, .usages = gfx::BindingUsage::computeWrite }
        }
    });
    assert(parameterBlockLayout);

    std::shared_ptr<gfx::ComputePipeline> pipeline = device.newComputePipeline({
        .computeShader = &pbrPrecomputeSlib.getFunction("convolveDiffuseIrradiance"),
        .threadsPerThreadgroupX = 8,
        .threadsPerThreadgroupY = 8,
        .threadsPerThreadgroupZ = 1,
        .parameterBlockLayouts = { parameterBlockLayout }
    });
    assert(pipeline);

    std::unique_ptr<gfx::ParameterBlockPool> parameterBlockPool = device.newParameterBlockPool({
        .maxBindingCount = {
            { gfx::BindingType::sampledTexture, 1 },
            { gfx::BindingType::storageTexture, 1 },
            { gfx::BindingType::sampler, 1 }
        }
    });
    assert(parameterBlockPool);

    std::shared_ptr<gfx::ParameterBlock> parameters = parameterBlockPool->get(parameterBlockLayout);
    parameters->setBinding(0, environmentCubemap);
    parameters->setBinding(1, sampler);
    parameters->setBinding(2, writableFaces);

    std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = device.newCommandBufferPool();
    assert(commandBufferPool);
    std::shared_ptr<gfx::CommandBuffer> commandBuffer = commandBufferPool->get();

    std::unique_ptr<gfx::ComputePassDescriptor> computePass = device.newComputePassDescriptor();
    commandBuffer->beginComputePass(*computePass);
    commandBuffer->usePipeline(pipeline);
    commandBuffer->setParameterBlock(parameters, 0);
    commandBuffer->dispatchThreadgroups(
        (irradianceFaceSize + 7) / 8,
        (irradianceFaceSize + 7) / 8,
        6
    );
    commandBuffer->endComputePass();

    device.submitCommandBuffers(commandBuffer);

    return irradianceCubemap;
}

std::shared_ptr<gfx::Texture> createPrefilteredEnvironmentCubemap(
    gfx::Device& device,
    gfx::ShaderLib& pbrPrecomputeSlib,
    const std::shared_ptr<gfx::Texture>& environmentCubemap
)
{
    assert(environmentCubemap);

    constexpr uint32_t faceSize = 128;
    constexpr uint32_t mipLevelCount = 5;

    std::shared_ptr<gfx::Texture> prefilteredCubemap = device.newTexture({
        .type = gfx::TextureType::textureCube,
        .width = faceSize,
        .height = faceSize,
        .mipLevelCount = mipLevelCount,
        .arrayLayerCount = 1,
        .pixelFormat = gfx::PixelFormat::RGBA16_float,
        .usages = gfx::TextureUsage::shaderRead | gfx::TextureUsage::shaderWrite,
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
    assert(prefilteredCubemap);

    std::shared_ptr<gfx::Sampler> sampler = device.newSampler({
        .sAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .tAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .rAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .minFilter = gfx::SamplerMinMagFilter::Linear,
        .magFilter = gfx::SamplerMinMagFilter::Linear,
        .mipFilter = gfx::SamplerMipFilter::Linear
    });
    assert(sampler);

    std::shared_ptr<gfx::ParameterBlockLayout> resourcePBlockLayout = device.newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::computeRead },
            { .type = gfx::BindingType::sampler, .usages = gfx::BindingUsage::computeRead },
            { .type = gfx::BindingType::storageTexture, .usages = gfx::BindingUsage::computeWrite }
        }
    });
    assert(resourcePBlockLayout);

    std::shared_ptr<gfx::ParameterBlockLayout> roughnessPBlockLayout = device.newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::constantBuffer, .usages = gfx::BindingUsage::computeRead }
        }
    });
    assert(roughnessPBlockLayout);

    std::shared_ptr<gfx::ComputePipeline> pipeline = device.newComputePipeline({
        .computeShader = &pbrPrecomputeSlib.getFunction("prefilterEnvironment"),
        .threadsPerThreadgroupX = 8,
        .threadsPerThreadgroupY = 8,
        .threadsPerThreadgroupZ = 1,
        .parameterBlockLayouts = { resourcePBlockLayout, roughnessPBlockLayout }
    });
    assert(pipeline);

    std::unique_ptr<gfx::ParameterBlockPool> parameterBlockPool = device.newParameterBlockPool({
        .maxBindingCount = {
            { gfx::BindingType::sampledTexture, mipLevelCount },
            { gfx::BindingType::storageTexture, mipLevelCount },
            { gfx::BindingType::sampler, mipLevelCount },
            { gfx::BindingType::constantBuffer, mipLevelCount }
        }
    });
    assert(parameterBlockPool);

    std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = device.newCommandBufferPool();
    assert(commandBufferPool);
    std::shared_ptr<gfx::CommandBuffer> commandBuffer = commandBufferPool->get();

    std::unique_ptr<gfx::ComputePassDescriptor> computePass = device.newComputePassDescriptor();
    commandBuffer->beginComputePass(*computePass);
    commandBuffer->usePipeline(pipeline);

    for (uint32_t mipLevel = 0; mipLevel < mipLevelCount; ++mipLevel)
    {
        std::shared_ptr<gfx::Texture> writableFaces = device.newTextureView(prefilteredCubemap, {
            .type = gfx::TextureType::texture2dArray,
            .baseMipLevel = mipLevel,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 6
        });
        assert(writableFaces);

        std::shared_ptr<gfx::Buffer> roughnessBuffer = device.newBuffer({
            .size = sizeof(float),
            .usages = gfx::BufferUsage::constantBuffer,
            .storageMode = gfx::ResourceStorageMode::hostVisible
        });
        assert(roughnessBuffer);
        *roughnessBuffer->content<float>() = static_cast<float>(mipLevel) / static_cast<float>(mipLevelCount - 1);

        std::shared_ptr<gfx::ParameterBlock> resourcePBlock = parameterBlockPool->get(resourcePBlockLayout);
        resourcePBlock->setBinding(0, environmentCubemap);
        resourcePBlock->setBinding(1, sampler);
        resourcePBlock->setBinding(2, writableFaces);

        std::shared_ptr<gfx::ParameterBlock> roughnessPBlock = parameterBlockPool->get(roughnessPBlockLayout);
        roughnessPBlock->setBinding(0, roughnessBuffer);

        commandBuffer->setParameterBlock(resourcePBlock, 0);
        commandBuffer->setParameterBlock(roughnessPBlock, 1);

        const uint32_t mipSize = std::max(faceSize >> mipLevel, 1u);
        commandBuffer->dispatchThreadgroups(
            (mipSize + 7) / 8,
            (mipSize + 7) / 8,
            6
        );
    }

    commandBuffer->endComputePass();
    device.submitCommandBuffers(commandBuffer);

    return prefilteredCubemap;
}

std::shared_ptr<gfx::Texture> createBrdfLut(gfx::Device& device, gfx::ShaderLib& pbrPrecomputeSlib)
{
    constexpr uint32_t lutSize = 512;

    std::shared_ptr<gfx::Texture> brdfLut = device.newTexture({
        .type = gfx::TextureType::texture2d,
        .width = lutSize,
        .height = lutSize,
        .mipLevelCount = 1,
        .arrayLayerCount = 1,
        .pixelFormat = gfx::PixelFormat::RG16_float,
        .usages = gfx::TextureUsage::shaderRead | gfx::TextureUsage::shaderWrite,
        .storageMode = gfx::ResourceStorageMode::deviceLocal
    });
    assert(brdfLut);

    std::shared_ptr<gfx::ParameterBlockLayout> parameterBlockLayout = device.newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::storageTexture, .usages = gfx::BindingUsage::computeWrite }
        }
    });
    assert(parameterBlockLayout);

    std::shared_ptr<gfx::ComputePipeline> pipeline = device.newComputePipeline({
        .computeShader = &pbrPrecomputeSlib.getFunction("integrateBrdf"),
        .threadsPerThreadgroupX = 8,
        .threadsPerThreadgroupY = 8,
        .threadsPerThreadgroupZ = 1,
        .parameterBlockLayouts = { parameterBlockLayout }
    });
    assert(pipeline);

    std::unique_ptr<gfx::ParameterBlockPool> parameterBlockPool = device.newParameterBlockPool({
        .maxBindingCount = {
            { gfx::BindingType::storageTexture, 1 }
        }
    });
    assert(parameterBlockPool);

    std::shared_ptr<gfx::ParameterBlock> parameters = parameterBlockPool->get(parameterBlockLayout);
    parameters->setBinding(0, brdfLut);

    std::unique_ptr<gfx::CommandBufferPool> commandBufferPool = device.newCommandBufferPool();
    assert(commandBufferPool);
    std::shared_ptr<gfx::CommandBuffer> commandBuffer = commandBufferPool->get();

    std::unique_ptr<gfx::ComputePassDescriptor> computePass = device.newComputePassDescriptor();
    commandBuffer->beginComputePass(*computePass);
    commandBuffer->usePipeline(pipeline);
    commandBuffer->setParameterBlock(parameters, 0);
    commandBuffer->dispatchThreadgroups((lutSize + 7) / 8, (lutSize + 7) / 8, 1);
    commandBuffer->endComputePass();

    device.submitCommandBuffers(commandBuffer);

    return brdfLut;
}

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

    auto environmentPrecomputeSlib = m_device->newShaderLib(SHADER_DIR "/equirectangularToCubemap.slib");
    auto irradiancePrecomputeSlib = m_device->newShaderLib(SHADER_DIR "/convolveDiffuseIrradiance.slib");
    auto prefilterPrecomputeSlib = m_device->newShaderLib(SHADER_DIR "/prefilterEnvironment.slib");
    auto brdfPrecomputeSlib = m_device->newShaderLib(SHADER_DIR "/integrateBrdf.slib");
    m_environmentCubemap = createEnvironmentCubemap(*m_device, *environmentPrecomputeSlib);
    m_irradianceCubemap = createIrradianceCubemap(*m_device, *irradiancePrecomputeSlib, m_environmentCubemap);
    m_prefilteredEnvironmentCubemap = createPrefilteredEnvironmentCubemap(*m_device, *prefilterPrecomputeSlib, m_environmentCubemap);
    m_brdfLut = createBrdfLut(*m_device, *brdfPrecomputeSlib);

    std::shared_ptr<gfx::ParameterBlockLayout> environmentLayout = m_device->newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::fragmentRead },
            { .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::fragmentRead },
            { .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::fragmentRead },
            { .type = gfx::BindingType::sampler, .usages = gfx::BindingUsage::fragmentRead }
        }
    });
    std::shared_ptr<gfx::ParameterBlockLayout> skyboxLayout = m_device->newParameterBlockLayout({
        .bindings = {
            { .type = gfx::BindingType::sampledTexture, .usages = gfx::BindingUsage::fragmentRead },
            { .type = gfx::BindingType::sampler, .usages = gfx::BindingUsage::fragmentRead }
        }
    });
    auto environmentPool = m_device->newParameterBlockPool({
        .maxBindingCount = {{gfx::BindingType::sampledTexture, 4}, {gfx::BindingType::sampler, 2}}
    });
    std::shared_ptr<gfx::Sampler> environmentSampler = m_device->newSampler({
        .sAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .tAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .rAddressMode = gfx::SamplerAddressMode::ClampToEdge,
        .minFilter = gfx::SamplerMinMagFilter::Linear,
        .magFilter = gfx::SamplerMinMagFilter::Linear,
        .mipFilter = gfx::SamplerMipFilter::Linear
    });
    m_environmentBlock = environmentPool->get(environmentLayout);
    m_environmentBlock->setBinding(0, m_irradianceCubemap);
    m_environmentBlock->setBinding(1, m_prefilteredEnvironmentCubemap);
    m_environmentBlock->setBinding(2, m_brdfLut);
    m_environmentBlock->setBinding(3, environmentSampler);
    m_skyboxBlock = environmentPool->get(skyboxLayout);
    m_skyboxBlock->setBinding(0, m_environmentCubemap);
    m_skyboxBlock->setBinding(1, environmentSampler);

    auto skyboxSlib = m_device->newShaderLib(SHADER_DIR "/skybox.slib");
    m_skyboxPipeline = m_device->newGraphicsPipeline({
        .vertexShader = &skyboxSlib->getFunction("vertexMain"),
        .fragmentShader = &skyboxSlib->getFunction("fragmentMain"),
        .colorAttachmentPxFormats = {gfx::PixelFormat::BGRA8_sRGB, gfx::PixelFormat::RG32Uint},
        .depthAttachmentPxFormat = gfx::PixelFormat::Depth32Float,
        .cullMode = gfx::CullMode::front,
        .depthCompareOperation = gfx::DepthCompareOperation::lessEqual,
        .parameterBlockLayouts = {skyboxLayout}
    });

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
        .magFilter = gfx::SamplerMinMagFilter::Linear,
        .mipFilter = gfx::SamplerMipFilter::Linear
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
                gfx::VertexAttribute{
                    .format = gfx::VertexAttributeFormat::float4,
                    .offset = offsetof(Vertex, tangent)},
            }
        },
        .vertexShader = &texturedShaderLib->getFunction("vertexMain"),
        .fragmentShader = &texturedShaderLib->getFunction("fragmentMain"),
        .colorAttachmentPxFormats = {gfx::PixelFormat::BGRA8_sRGB, gfx::PixelFormat::RG32Uint},
        .depthAttachmentPxFormat = gfx::PixelFormat::Depth32Float,
        .blendOperation = gfx::BlendOperation::blendingOff,
        .cullMode = gfx::CullMode::back,
        .parameterBlockLayouts = {
            textureTablePBLayout,
            m_frameDataBlockLayout,
            m_texturedMaterialPBlockLayout,
            environmentLayout
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
            .device = *m_device,
            .assetManager = *m_assetManager,
            .commandBuffer = *commandBuffer,
            .parameterBlockPool = *cfd.parameterBlockPool,
            .textureTableBlock = m_textureTableBlock,
            .frameDataBlockLayout = m_frameDataBlockLayout,
            .texture = [&](FrameGraph::TextureRef ref){ return textureMap.at(ref); },
            .buffer = [&](FrameGraph::BufferRef ref){ return bufferMap.at(ref); },
            .texturedMaterialPBlockLayout = m_texturedMaterialPBlockLayout,
            .texturedPipeline = m_texturedPipeline,
            .environmentBlock = m_environmentBlock,
            .skyboxBlock = m_skyboxBlock,
            .skyboxPipeline = m_skyboxPipeline
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

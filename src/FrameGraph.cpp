/*
 * ---------------------------------------------------
 * FrameGraph.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/FrameGraph.hpp"
#include "Graphics/Enums.hpp"

#include <Graphics/Texture.hpp>
#include <Graphics/Buffer.hpp>

#include <stdexcept>

namespace GE
{

FrameGraph::FrameGraph(const Descriptor& desc)
    : m_passes(desc.passes)
    , m_backBufferName(desc.backBufferName)
{
    // Process texture declarations - all go into m_textureDescriptors including back buffer
    for (const TextureDescriptor& texture : desc.textures)
    {
        const gfx::Texture::Descriptor newDescriptor{
            .width = texture.size.first,
            .height = texture.size.second,
            .pixelFormat = texture.pixelFormat,
            .usages = {},
        };

        auto [it, inserted] = m_textureDescriptors.emplace(texture.name, newDescriptor);
        if (!inserted)
            throw std::runtime_error("Duplicate texture declaration for \"" + texture.name + "\"");
    }

    // Add usage flags based on how passes reference the textures
    auto addUsageToTexture = [&](const std::string& name, gfx::TextureUsages usage) {
        auto it = m_textureDescriptors.find(name);
        if (it == m_textureDescriptors.end())
            throw std::runtime_error("Texture \"" + name + "\" is used but not declared");
        it->second.usages |= usage;
    };

    for (const FramePass& pass : m_passes)
    {
        for (const AttachmentDescriptor& colorAttachment : pass.colorAttachments)
            addUsageToTexture(colorAttachment.texture, gfx::TextureUsage::colorAttachment);

        if (pass.depthAttachment)
            addUsageToTexture(pass.depthAttachment->texture, gfx::TextureUsage::depthStencilAttachment);

        for (const std::string& sampledTexture : pass.sampledTextures)
            addUsageToTexture(sampledTexture, gfx::TextureUsage::shaderRead);
    }

    // Process constant buffer declarations
    for (const ConstantBufferDescriptor& cbDesc : desc.constantBuffers)
    {
        gfx::Buffer::Descriptor bufferDesc{
            .size = cbDesc.size,
            .usages = gfx::BufferUsage::constantBuffer,
            .storageMode = gfx::ResourceStorageMode::hostVisible
        };

        auto [it, inserted] = m_constantBufferDescriptors.emplace(cbDesc.name, bufferDesc);
        if (!inserted)
            throw std::runtime_error("Duplicate constant buffer declaration for \"" + cbDesc.name + "\"");
    }

    // Process structured buffer declarations
    for (const StructuredBufferDescriptor& sbDesc : desc.structuredBuffers)
    {
        auto [it, inserted] = m_structuredBufferNames.insert(sbDesc.name);
        if (!inserted)
            throw std::runtime_error("Duplicate structured buffer declaration for \"" + sbDesc.name + "\"");
    }
}

}

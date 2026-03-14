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
    // Process attachment declarations - all go into m_textureDescriptors including back buffer
    for (const AttachmentDescriptor& attachment : desc.attachments)
    {
        const gfx::Texture::Descriptor newDescriptor{
            .width = attachment.size.first,
            .height = attachment.size.second,
            .pixelFormat = attachment.pixelFormat,
            .usages = {},
        };

        auto [it, inserted] = m_textureDescriptors.emplace(attachment.name, newDescriptor);
        if (!inserted)
            throw std::runtime_error("Duplicate attachment declaration for \"" + attachment.name + "\"");
    }

    // Add usage flags based on how passes reference the attachments
    auto addUsageToAttachment = [&](const std::string& name, gfx::TextureUsages usage) {
        auto it = m_textureDescriptors.find(name);
        if (it == m_textureDescriptors.end())
            throw std::runtime_error("Attachment \"" + name + "\" is used but not declared");
        it->second.usages |= usage;
    };

    for (const FramePass& pass : m_passes)
    {
        for (const AttachmentUsage& colorAttachment : pass.colorAttachments)
            addUsageToAttachment(colorAttachment.name, gfx::TextureUsage::colorAttachment);

        if (pass.depthAttachment)
            addUsageToAttachment(pass.depthAttachment->name, gfx::TextureUsage::depthStencilAttachment);

        for (const std::string& sampledAttachment : pass.sampledAttachments)
            addUsageToAttachment(sampledAttachment, gfx::TextureUsage::shaderRead);
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

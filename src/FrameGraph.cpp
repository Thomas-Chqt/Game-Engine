/*
 * ---------------------------------------------------
 * FrameGraph.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/FrameGraph.hpp"

#include <Graphics/Texture.hpp>

#include <stdexcept>
#include <utility>

namespace GE
{

FrameGraph::FrameGraph(const Descriptor& desc)
    : m_passes(desc.passes)
    , m_backBufferDescriptor(std::make_pair(desc.backBufferName, gfx::Texture::Descriptor{}))
{
    auto addAttachment = [&](const AttachmentDescriptor& attachment, gfx::TextureUsage usage) {
        const gfx::Texture::Descriptor newDescriptor{
            .width = attachment.size.first,
            .height = attachment.size.second,
            .pixelFormat = attachment.pixelFormat,
            .usages = usage,
        };

        auto mergeDescriptor = [&](gfx::Texture::Descriptor& descriptor, const std::string& name) {
            if (descriptor.width != newDescriptor.width || descriptor.height != newDescriptor.height || descriptor.pixelFormat != newDescriptor.pixelFormat)
                throw std::runtime_error("Conflicting texture descriptor for attachment \"" + name + "\"");
            descriptor.usages |= newDescriptor.usages;
        };

        if (attachment.name == m_backBufferDescriptor.first)
        {
            auto& descriptor = m_backBufferDescriptor.second;
            if (descriptor.width == 0 && descriptor.height == 0)
                descriptor = newDescriptor;
            else
                mergeDescriptor(descriptor, attachment.name);
            return;
        }

        auto [it, inserted] = m_textureDescriptors.emplace(attachment.name, newDescriptor);
        if (!inserted)
            mergeDescriptor(it->second, attachment.name);
    };

    for (const FramePass& pass : m_passes)
    {
        for (const AttachmentDescriptor& colorAttachment : pass.colorAttachments)
            addAttachment(colorAttachment, gfx::TextureUsage::colorAttachment);

        if (pass.depthAttachment)
            addAttachment(*pass.depthAttachment, gfx::TextureUsage::depthStencilAttachment);
    }
}

}
